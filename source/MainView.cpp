#include "MainView.h"
#include <Path.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <Directory.h>
#include <File.h>
#include <BitmapStream.h>
#include <string.h>
#include <NodeInfo.h>
#include <Screen.h>
#include <Application.h>
#include "main.h"
#include <stdlib.h>
//--------------------------------------------------------------------
#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)>(b)?(a):(b))
//--------------------------------------------------------------------
//				from ShowImage
//--------------------------------------------------------------------

const rgb_color kAlphaLow = (rgb_color) { 0xbb, 0xbb, 0xbb, 0xff };
const rgb_color kAlphaHigh = (rgb_color) { 0xe0, 0xe0, 0xe0, 0xff };

inline void
blend_colors(uint8* d, uint8 r, uint8 g, uint8 b, uint8 a)
{
	d[0] = ((b - d[0]) * a + (d[0] << 8)) >> 8;
	d[1] = ((g - d[1]) * a + (d[1] << 8)) >> 8;
	d[2] = ((r - d[2]) * a + (d[2] << 8)) >> 8;
}

BBitmap*
compose_checker_background(const BBitmap* bitmap)
{
	BBitmap* result = new (nothrow) BBitmap(bitmap);
	if (result && !result->IsValid()) {
		delete result;
		result = NULL;
	}
	if (!result)
		return NULL;

	uint8* bits = (uint8*)result->Bits();
	uint32 bpr = result->BytesPerRow();
	uint32 width = result->Bounds().IntegerWidth() + 1;
	uint32 height = result->Bounds().IntegerHeight() + 1;

	for (uint32 i = 0; i < height; i++) {
		uint8* p = bits;
		for (uint32 x = 0; x < width; x++) {
			uint8 alpha = p[3];
			if (alpha < 255) {
				p[3] = 255;
				alpha = 255 - alpha;
				if (x % 10 >= 5) {
					if (i % 10 >= 5)
						blend_colors(p, kAlphaLow.red, kAlphaLow.green, kAlphaLow.blue, alpha);
					else
						blend_colors(p, kAlphaHigh.red, kAlphaHigh.green, kAlphaHigh.blue, alpha);
				} else {
					if (i % 10 >= 5)
						blend_colors(p, kAlphaHigh.red, kAlphaHigh.green, kAlphaHigh.blue, alpha);
					else
						blend_colors(p, kAlphaLow.red, kAlphaLow.green, kAlphaLow.blue, alpha);
				}
			}
			p += 4;
		}
		bits += bpr;
	}
	return result;
}
//--------------------------------------------------------------------
MainView::MainView() 
	: BView(BRect(0, 0, 200, 100), "Image", B_FOLLOW_NONE, B_WILL_DRAW)
{
	SetViewColor(128, 128, 128);
	OriginalBitmap = NULL; //on store le bitmap qu'on recoit
	ModifiedBitmap = NULL; //on store le out bitmap
	offscreenBitmap = NULL; //pour eviter le flickering
	SetViewColor(B_TRANSPARENT_32_BIT);
	DontResize = false;
	KeepRatio = true;
	OriginalBitmap = NULL;

	//pas d'output selectionne
	dragging = false;
	all_translators = NULL;
	CurrentTranslator = -1;
	CurrentOutput = -1;
	Clipping1 = BPoint(-1,-1);
	Clipping2 = BPoint(-1,-1);
}
//----------------------------------------------------------------------
MainView::~MainView()
{
	if(OriginalBitmap != NULL) 
	{
		delete OriginalBitmap;
		delete FirstBitmap;
	}
}
//----------------------------------------------------------------------
BRect MainView::GetRegion()
{//retourne la region selectionne entre clipping 1 et 2
	return BRect(min(Clipping1.x, Clipping2.x), min(Clipping1.y, Clipping2.y),
				 max(Clipping1.x, Clipping2.x), max(Clipping1.y, Clipping2.y));
}
//----------------------------------------------------------------------
bool MainView::GetImage(const char* path)
{
	if(OriginalBitmap != NULL) 
	{
		delete OriginalBitmap;
		delete FirstBitmap;
	}

	BBitmap* AnyImageFormat = BTranslationUtils::GetBitmap(path);
	if(AnyImageFormat == NULL) return false;

	//on doit convertir le bitmap lu en RGB_32 pour que nos effets fonctionnent...
	BRect B = AnyImageFormat->Bounds();
	OriginalBitmap = new BBitmap(B, B_RGBA32, true);
	offscreenView = new BView(B, "", B_FOLLOW_NONE, (uint32)NULL);
	OriginalBitmap->AddChild(offscreenView);
	OriginalBitmap->Lock(); 
	offscreenView->DrawBitmap(AnyImageFormat, B);
	offscreenView->Sync(); 
	OriginalBitmap->Unlock();
	OriginalBitmap->RemoveChild(offscreenView);		
	delete AnyImageFormat;

	FirstBitmap = new BBitmap(OriginalBitmap);
	Ratio = OriginalBitmap->Bounds().right / OriginalBitmap->Bounds().bottom;

	//on resize la fenetre sans recalculer la view a l'interieur
	DontResize = true;
	ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom); //grosseur originale
	Window()->ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom);
	Flush(); //flush les vieux undo...
	return true;
}
//----------------------------------------------------------------------
void MainView::ResizeImage()
{
	//On anule le clipping region si il y en a une
	Clipping1 = BPoint(-1,-1);
	Clipping2 = BPoint(-1,-1);
	
	if(OriginalBitmap == NULL) //pas d'image
	{//la fenetre est resize, on agrandit la view pour prendre tout l'espace dispo.
		BRect W = Window()->Bounds();
		ResizeTo(W.right, W.bottom);
		Invalidate();
	}

	else if(DontResize)
	{	//cas special pour le drop pour pas caller le resize
		DontResize = false;
		Invalidate(); //on fait rien, juste redessiner sans rien modifier
	}

	else if(KeepRatio)
	{//resize la view pour quelle prenne le plus de place en gardant le ratio intact
		BRect Fenetre = Window()->Bounds();

		//ratio = Largeur / Hauteur
		if(Fenetre.bottom >= Fenetre.right / Ratio)
		{
			ResizeTo(Fenetre.right, Fenetre.right / Ratio); //ajustement en fonction de la largeur
			Window()->ResizeTo(Fenetre.right, Fenetre.right / Ratio);
		}
		else
		{
			ResizeTo(Fenetre.bottom * Ratio, Fenetre.bottom); //ajustement en fonction de la hauteur
			Window()->ResizeTo(Fenetre.bottom * Ratio, Fenetre.bottom);
		}
		Invalidate();
	}	
	
	else
	{
		BRect W = Window()->Bounds();
		ResizeTo(W.right, W.bottom);
		Invalidate();
	}
 
	if(((Resizer*)be_app)->Fenetre->DontUpdate)
	{//le resize a ete cause par une entree manuelle dans la fenetre option, on reponds pas
		((Resizer*)be_app)->Fenetre->DontUpdate = false;
		return;
	}

	BMessage* message = new BMessage(TEXT_WIDTH);
	message->AddInt16("NewWidth", Bounds().right+1);
	((Resizer*)be_app)->Option->PostMessage(message);

	BMessage* message2 = new BMessage(TEXT_HEIGHT);
	message2->AddInt16("NewHeight", Bounds().bottom+1);
	((Resizer*)be_app)->Option->PostMessage(message2);
}
//----------------------------------------------------------------------
void MainView::ResizeImage(int width, int height)
{//l'usager a entre des valeurs manuellement, on traite l'input de l'usager
	if(OriginalBitmap == NULL) //pas d'image
	{
		int w = width;
		int h = height;
		if(w == -1) w = (int)Bounds().right - 1; //on garde la meme valeur
		else if(h == -1) h = (int)Bounds().bottom - 1; //on garde la meme valeur
		Window()->ResizeTo(w, h); //resize la fenetre (on veut juste pas planter)
	}

	else if(KeepRatio)
	{//on resize la view en accord avec ce que veut l'usager
		int W = 0;
		int H = 0;

		if(width == -1)
		{
			H = height-1;
			W = (int)(Ratio * H);

			BMessage* message = new BMessage(TEXT_WIDTH);
			message->AddInt16("NewWidth", W+1);
			((Resizer*)be_app)->Option->PostMessage(message);
		}

		else if(height == -1)
		{
			W = width-1;
			H = (int)(W / Ratio);		

			BMessage* message2 = new BMessage(TEXT_HEIGHT);
			message2->AddInt16("NewHeight", H+1);
			((Resizer*)be_app)->Option->PostMessage(message2);
		}

		DontResize = true; //pas resizer la view en resizant la fenetre
		ResizeTo(W, H); //ajustement de l'image
		Window()->ResizeTo(W, H); //on fait fitter l'image autour
		Invalidate();
	}	
	
	else
	{//on garde pas le ratio, on fait juste foutre la nouvelle valeur dedans
		int W = 0;
		int H = 0;

		if(width == -1)
		{
			H = height-1;
			W = (int)Bounds().right;
		}

		else if(height == -1)
		{
			W = width-1;
			H = (int)Bounds().bottom;		
		}

		DontResize = true; //pas resizer la view en resizant la fenetre
		ResizeTo(W, H); //ajustement de l'image
		Window()->ResizeTo(W, H); //on fait fitter l'image autour
		Invalidate();
	}
}
//----------------------------------------------------------------------
void MainView::Draw(BRect R)
{
	BRect B = Bounds();
	if(OriginalBitmap == NULL)
	{//on fait un petit fill bon chic bon genre
		BRect bounds = B;
		const pattern stripePattern = {0xcc, 0x66, 0x33, 0x99, 0xcc, 0x66, 0x33, 0x99};
		const char *stringMessage = "Drag and drop an image";

		font_height fontHeight;
		GetFontHeight(&fontHeight);
		const char* text = stringMessage;

		float width = StringWidth(text);

		BString truncated;
		if (width - 10 > bounds.Width()) {
			truncated = stringMessage;
			TruncateString(&truncated, B_TRUNCATE_END, bounds.Width());
			text = truncated.String();
			width = StringWidth(text);
		}

		float y = (bounds.top + bounds.bottom - ceilf(fontHeight.ascent)
			- ceilf(fontHeight.descent)) / 2.0 + ceilf(fontHeight.ascent);
		float x = (bounds.Width() - width) / 2.0;

		SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(bounds);

		SetDrawingMode(B_OP_ALPHA);
		SetLowColor(0, 0, 0, 0);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
			B_DARKEN_2_TINT));

		FillRect(bounds, B_SOLID_LOW);
		StrokeRect(bounds);
		FillRect(bounds.InsetBySelf(3, 3), stripePattern);

		BRect labelBackground(x - 5, y - ceilf(fontHeight.ascent),
			x + width + 5, y + ceilf(fontHeight.descent));
		SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(labelBackground, B_SOLID_LOW);
		SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
		DrawString(text, BPoint(x, y));

		BView::Draw(R);
		return;
	}
	delete offscreenBitmap;
	offscreenBitmap  = new BBitmap(B, B_RGBA32, true);
	offscreenView = new BView(B, "", B_FOLLOW_NONE, (uint32)NULL);
	offscreenBitmap->AddChild(offscreenView);

	offscreenBitmap->Lock(); //protege le offscreenBitmap contre les intrusions

	((Resizer*)be_app)->Option->Lock();
	bool smoothScaling = ((Resizer*)be_app)->Option->Option->Smooth->Value() != 0;
	((Resizer*)be_app)->Option->Option->Sync();
	((Resizer*)be_app)->Option->Unlock();

	if (!smoothScaling || (OriginalBitmap->Bounds().Width() <= B.Width()
		&& OriginalBitmap->Bounds().Height() <= B.Height()))
			offscreenView->DrawBitmap(OriginalBitmap, OriginalBitmap->Bounds(), B, smoothScaling ? B_FILTER_BITMAP_BILINEAR : 0);
	else {
		SmoothScale(OriginalBitmap, offscreenBitmap);
	}
	//Draw the invalidated portion of the offscreen bitmap into the onscreen view
	offscreenView->Sync(); //Synchronise la vue offscreen
	DrawBitmap(compose_checker_background(offscreenBitmap));
//	DrawBitmap(offscreenBitmap); //Copie le bitmap a l'ecran
	offscreenBitmap->Unlock(); //delock le bitmap offscreen
	offscreenBitmap->RemoveChild(offscreenView);		

	if(Clipping1 != BPoint(-1,-1) && Clipping2 != BPoint(-1,-1))
	{
		SetHighColor(255, 0, 255, 255);
		StrokeRect(GetRegion());
		SetHighColor(255, 255, 255, 255);
	}

	delete offscreenView;
	//delete offscreenBitmap;
}
//----------------------------------------------------------------------
void MainView::MessageReceived(BMessage *message)
{
   	entry_ref ref;
	BPath path;
		
 	switch (message->what)
 	{
   		case B_SIMPLE_DATA:
   		{// Look for a ref in the message
   			if(message->FindRef("refs", &ref) == B_OK)
   			{// Call SetText() to change the string in the view
				BEntry Entry(&ref);
				Entry.GetPath(&path);
				((Resizer*)be_app)->Option->Lock();
				((Resizer*)be_app)->Option->Option->FileName->SetText(ref.name);
				((Resizer*)be_app)->Option->Option->Sync();
				((Resizer*)be_app)->Option->Unlock();
				GetImage(path.Path());
			}
   		
   			else BView::MessageReceived(message);break;
   		}
   		default:BView::MessageReceived(message);break;
	}
}
//----------------------------------------------------------------------
void MainView::ToggleRatio()
{
	KeepRatio = !KeepRatio;
	ResizeImage();
}
//----------------------------------------------------------------------
void MainView::ResetImage()
{
	if(OriginalBitmap == NULL) return;
	delete OriginalBitmap;
	OriginalBitmap = new BBitmap(FirstBitmap);
	Ratio = OriginalBitmap->Bounds().right / OriginalBitmap->Bounds().bottom;
	KeepRatio = true;
	DontResize = true; //pour pas caller de resize de la view en partant
	ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom);
	Window()->ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom);
	Invalidate();
}
//--------------------------------------------------------------------------------
void MainView::MouseDown(BPoint where) 
{//on clique qq sur l'image
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if(buttons == B_SECONDARY_MOUSE_BUTTON) //Right-Click
	{//on veut definir une region
		Clipping1 = where; //on sauve le point d'origine
		dragging = true;

		BMessage* Message = new BMessage(CLIP1);
		Message->AddPoint("Clip1", Clipping1);
		((Resizer*)be_app)->Mouse->PostMessage(Message);

		return;
	}

	else if(!BRect(GetRegion()).Contains(where))
	{//click normal on deselectionne
		Clipping1 = BPoint(-1,-1); //on enleve la selection si on clique a cote
		Clipping2 = BPoint(-1,-1);
		Invalidate();

		BMessage* Message = new BMessage(CLIP0);
		((Resizer*)be_app)->Mouse->PostMessage(Message);
	}

	//on commence a dragger l'image ou une partie de l'image
	if(OriginalBitmap == NULL || CurrentTranslator == -1)
		return; //pas d'image ou pas d'output choisit

	BPoint P;
	uint32 mod;
	while(true)
	{
		GetMouse(&P, &mod);
		if(!mod) //aucun bouton enfonce
		{Window()->Activate(true); return;}
		
		if(P != where) break; //on commence a bouger
		snooze(40000); //on attends un peu
	}

	BMessage Message(B_SIMPLE_DATA);
	Message.AddString("be:types", B_FILE_MIME_TYPE);

	const translation_format* FormatOut;
	int32 FormatOutCount;
	BTranslatorRoster::Default()->GetOutputFormats(all_translators[CurrentTranslator], &FormatOut, &FormatOutCount);
	Message.AddString("be:types", FormatOut[CurrentOutput].MIME);
	Message.AddString("be:filetypes", FormatOut[CurrentOutput].MIME);
	Message.AddInt32("be:actions", B_COPY_TARGET);

	((Resizer*)be_app)->Option->Lock();
	Message.AddString("be:clip_name", ((Resizer*)be_app)->Option->Option->FileName->Text());
	((Resizer*)be_app)->Option->Option->Sync();
	((Resizer*)be_app)->Option->Unlock();

	DragMessage(&Message, BRect(where.x-10, where.y-10, where.x+10, where.y+10), Window());
}
//--------------------------------------------------------------------------------
void MainView::MouseUp(BPoint where)
{
	if(dragging) //on est en train de sizer une region
	{	
		dragging = false;
		BPoint P;
		uint32 mod;
		GetMouse(&P, &mod);
		Clipping2 = P; //on sauve le point d'origine
		Invalidate();

		BMessage* Message = new BMessage(CLIP2);
		Message->AddPoint("Clip2", Clipping2);
		((Resizer*)be_app)->Mouse->PostMessage(Message);
	}
}
//--------------------------------------------------------------------------------
void MainView::MouseMoved(BPoint point, uint32 transit, const BMessage* message)
{
	BMessage* Message = new BMessage(MOVEDPOINT);
	Message->AddPoint("MovedPoint", point);
	((Resizer*)be_app)->Mouse->PostMessage(Message);

	if(dragging)
		if(Clipping2 != point)
		{
			Clipping2 = point;
			Invalidate();

			BMessage* Message = new BMessage(CLIP2);
			Message->AddPoint("Clip2", Clipping2);
			((Resizer*)be_app)->Mouse->PostMessage(Message);
		}
}
//--------------------------------------------------------------------------------
void MainView::Copy(BMessage * request)
{
	if(Clipping1 != BPoint(-1,-1) && Clipping2 != BPoint(-1,-1))
	{//on copie le clipping
		BRect Source = GetRegion();
		BRect Destination = BRect(0,0,Source.right-Source.left, Source.bottom-Source.top);
		
		//Image temporaire que l'on size correctement
		BBitmap* temp = new BBitmap(Bounds(), B_RGBA32, true);
		BView* OffView = new BView(Bounds(), "", B_FOLLOW_NONE, (uint32)NULL);
		temp->AddChild(OffView);
		temp->Lock();
		OffView->DrawBitmap(offscreenBitmap, Bounds());
		OffView->Sync();
		temp->Unlock();
		temp->RemoveChild(OffView);		
		delete OffView;

		//applique le clipping sur cette image resizee
		ModifiedBitmap = new BBitmap(Destination, B_RGBA32, true);
		OffView = new BView(Destination, "", B_FOLLOW_NONE, (uint32)NULL);
		ModifiedBitmap->AddChild(OffView);
		ModifiedBitmap->Lock();
		OffView->DrawBitmap(temp, Source, Destination);
		OffView->Sync();
		ModifiedBitmap->Unlock();
		ModifiedBitmap->RemoveChild(OffView);		
		delete OffView;
		delete temp;
	}
	
	else
	{//on copie l'image au complet
		ModifiedBitmap = new BBitmap(Bounds(), B_RGBA32, true);
		BView* OffView = new BView(Bounds(), "", B_FOLLOW_NONE, (uint32)NULL);
		ModifiedBitmap->AddChild(OffView);
		ModifiedBitmap->Lock();
		OffView->DrawBitmap(offscreenBitmap, Bounds());
		OffView->Sync();
		ModifiedBitmap->Unlock();
		ModifiedBitmap->RemoveChild(OffView);		
		delete OffView;
	}

	const char* type = NULL;
	if(!request->FindString("be:types", &type))
	{
		BBitmapStream stream(ModifiedBitmap);
		const translation_format* FormatOut;
		int32 FormatOutCount;
		BTranslatorRoster::Default()->GetOutputFormats(all_translators[CurrentTranslator], &FormatOut, &FormatOutCount);

		if(!strcasecmp(type, B_FILE_MIME_TYPE))
		{
			const char * name;
			entry_ref dir;
			if(!request->FindString("be:filetypes", &type) && !request->FindString("name", &name) 
			&& !request->FindRef("directory", &dir)) 
			{//	write file
				BDirectory d(&dir);
				BFile f(&d, name, O_RDWR | O_TRUNC);
				BTranslatorRoster::Default()->Translate(all_translators[CurrentTranslator],
				&stream, NULL, &f, FormatOut[CurrentOutput].type);
				BNodeInfo ni(&f);
				ni.SetType(FormatOut[CurrentOutput].MIME);
			}
		}

		else 
		{//bonne question a savoir si ca marche...
			BMessage msg(B_MIME_DATA);
			BMallocIO f;
			BTranslatorRoster::Default()->Translate(all_translators[CurrentTranslator],
					&stream, NULL, &f, FormatOut[CurrentOutput].type);
			msg.AddData(FormatOut[CurrentOutput].MIME, B_MIME_TYPE, f.Buffer(), f.BufferLength());
			request->SendReply(&msg);
		}
		stream.DetachBitmap(&ModifiedBitmap);
	}
	delete ModifiedBitmap;
}

/*===================================================================
//UNDO
===================================================================*/
void MainView::AddBitmap(BBitmap* B)
{
	file.push_back(B);
	if(file.size() > 8)
	{//on efface le plus vieux
		delete file.front();
		file.pop_front();
	}

	BMessage* message = new BMessage(UNDOOK);
	((Resizer*)be_app)->Option->PostMessage(message);
}
//-------------------------------------------------------------------
void MainView::Undo()
{
	if(file.empty()) return;
	delete OriginalBitmap;
	OriginalBitmap = file.back();
	file.pop_back(); //l'enleve de la liste
	Invalidate();
	if(file.empty())
	{
		BMessage* message = new BMessage(UNDONOK);
		((Resizer*)be_app)->Option->PostMessage(message);
	}
}
//-------------------------------------------------------------------
void MainView::Flush()
{
	while(!file.empty())
	{
		delete file.back();
		file.pop_back();
	}

	BMessage* message = new BMessage(UNDONOK);
	((Resizer*)be_app)->Option->PostMessage(message);
}

/*===================================================================
//EFFECTS
===================================================================*/
void MainView::RotateImage()
{//va creer un nouvelle image rotatee de 90 degree.
	if(OriginalBitmap == NULL) return;
	BRect coord = BRect(0, 0, OriginalBitmap->Bounds().bottom, OriginalBitmap->Bounds().right);
	BBitmap* Spin = new BBitmap(coord, B_RGBA32, true);

	int widthO = (int)OriginalBitmap->Bounds().right+1;
	int heightO = (int)OriginalBitmap->Bounds().bottom+1;
	int widthD = (int)Spin->Bounds().right+1;

	for(int row = 0; row < heightO; row++)
		for(int col = 0; col < widthO; col++)
		{
			((rgb_color *)Spin->Bits())[(col*widthD + row)] = 
			((rgb_color *)OriginalBitmap->Bits())[row*widthO + widthO - col];
		}			

	BRect temp = OriginalBitmap->Bounds();
	delete OriginalBitmap;
	OriginalBitmap = Spin;
	Ratio = OriginalBitmap->Bounds().right / OriginalBitmap->Bounds().bottom;
	ResizeImage(temp.bottom+1, -1); 
	ResizeImage(-1, temp.right+1); 
	Flush();
}
//-------------------------------------------------------------------
void MainView::Flip(bool horizontal)
{
	if(OriginalBitmap == NULL) return;
	Clipping1 = BPoint(-1,-1);
	Clipping2 = BPoint(-1,-1);
	BBitmap* Flip = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	if(horizontal)
		for(int row = 0; row < height; row++)
			for(int col = 0; col < width; col++)
			{
				((rgb_color *)Flip->Bits())[((height-1-row)*widthSize + col)] = 
				((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			}			

	else //vertical
		for(int row = 0; row < height; row++)
			for(int col = 0; col < width; col++)
			{
				((rgb_color *)Flip->Bits())[(row*widthSize + width-1-col)] = 
				((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			}			
	
	AddBitmap(OriginalBitmap);
	OriginalBitmap = Flip;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::Dark() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Darker = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color CurrentColor;
	int r, g, b;
	Window()->SetTitle("Working...");
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			CurrentColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			r = CurrentColor.red - 8;
			g = CurrentColor.green - 8;
			b = CurrentColor.blue - 8;
			if(r < 0) r=0;
			if(g < 0) g=0;
			if(b < 0) b=0;
			((rgb_color *)Darker->Bits())[row*widthSize + col].red = r; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].green = g; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].blue = b;
			((rgb_color *)Darker->Bits())[row*widthSize + col].alpha = CurrentColor.alpha; 
		}			
	
	AddBitmap(OriginalBitmap);
	OriginalBitmap = Darker;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::Light() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Darker = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color CurrentColor;
	int r, g, b;
	Window()->SetTitle("Working...");
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			CurrentColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			r = CurrentColor.red + 8;
			g = CurrentColor.green + 8;
			b = CurrentColor.blue + 8;
			if(r > 255) r=255;
			if(g > 255) g=255;
			if(b > 255) b=255;
			((rgb_color *)Darker->Bits())[row*widthSize + col].red = r; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].green = g; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].blue = b; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].alpha = CurrentColor.alpha; 
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Darker;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::Blur()
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Blured = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	
	rgb_color CC; //current color
	rgb_color T; //top color
	rgb_color L; //left color
	rgb_color R; //right color
	rgb_color D; //down color
	rgb_color TR; //top right color
	rgb_color TL; //top left color
	rgb_color BL; //down left color
	rgb_color BR; //down right color
	rgb_color final; //final color
	rgb_color empty; empty.red=0; empty.green=0; empty.blue=0; empty.alpha=255;//no color
	int contour;

	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			contour = 0;
			CC = ((rgb_color *)OriginalBitmap->Bits())[(row)*widthSize + col];

			if(row > 0) {T = ((rgb_color *)OriginalBitmap->Bits())[(row-1)*widthSize + col]; contour++;}
			else T = empty;

			if(col > 0) {L = ((rgb_color *)OriginalBitmap->Bits())[(row)*widthSize + col-1]; contour++;}
			else L = empty;

			if(col < width - 1)	{R = ((rgb_color *)OriginalBitmap->Bits())[(row)*widthSize + col+1]; contour++;}
			else R = empty;
			
			if(row < height - 1) {D = ((rgb_color *)OriginalBitmap->Bits())[(row+1)*widthSize + col]; contour++;}
			else D = empty;
			
			if(row > 0 && col > 0) {TL = ((rgb_color *)OriginalBitmap->Bits())[(row-1)*widthSize + col-1]; contour++;}
			else TL = empty;
			
			if(row > 0 && col < width - 1) {TR = ((rgb_color *)OriginalBitmap->Bits())[(row-1)*widthSize + col+1]; contour++;}
			else TR = empty;

			if(row < height - 1 && col > 0) {BL = ((rgb_color *)OriginalBitmap->Bits())[(row+1)*widthSize + col-1]; contour++;}
			else BL = empty;
			
			if(row < height - 1 && col < width - 1) {BR = ((rgb_color *)OriginalBitmap->Bits())[(row+1)*widthSize + col+1]; contour++;}
			else BR = empty;

			final.red = (int)((((L.red + R.red + T.red + D.red + TL.red + TR.red + BL.red + BR.red)
				/(4*contour))) + (CC.red * 0.75));
			final.green = (int)((((L.green + R.green + T.green + D.green + TL.green + TR.green + BL.green + BR.green)
				/(4*contour))) + (CC.green * 0.75));
			final.blue = (int)((((L.blue + R.blue + T.blue + D.blue + TL.blue + TR.blue + BL.blue + BR.blue)
				/(4*contour))) + (CC.blue * 0.75));
			final.alpha = CC.alpha;

			((rgb_color *)Blured->Bits())[row*widthSize + col] = final;
		}

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Blured;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::GrabScreen()
{//fait un screen shot de l'ecran
	//on cache nos fenetres
	((Resizer*)be_app)->Option->Lock();
	((Resizer*)be_app)->Option->Option->FileName->SetText("screenshot");
	((Resizer*)be_app)->Option->Hide();
	((Resizer*)be_app)->Option->Option->Sync();
	((Resizer*)be_app)->Option->Unlock();
	Window()->Hide();

	if(OriginalBitmap != NULL) 
	{
		delete OriginalBitmap;
		delete FirstBitmap;
	}

	//On grab Maintenant, phase critique, la config de l'ecran est bloque !!!
	BBitmap* AnyImageFormat;
	BScreen* TheScreen = new BScreen();
	snooze(500000);
	TheScreen->GetBitmap(&AnyImageFormat);
	delete TheScreen;
	//Ouf, c'est fait, tout est redebloque...

	//on doit convertir le bitmap lu en RGB_32 pour que nos effets fonctionnent...
	BRect B = AnyImageFormat->Bounds();
	OriginalBitmap = new BBitmap(B, B_RGBA32, true);
	offscreenView = new BView(B, "", B_FOLLOW_NONE, (uint32)NULL);
	OriginalBitmap->AddChild(offscreenView);
	OriginalBitmap->Lock(); 
	offscreenView->DrawBitmap(AnyImageFormat, B);
	offscreenView->Sync(); 
	OriginalBitmap->Unlock();
	OriginalBitmap->RemoveChild(offscreenView);		
	delete AnyImageFormat;
	// fill alpha channel
	uint8* pixel = (uint8*)OriginalBitmap->Bits();
	uint32 count = OriginalBitmap->BitsLength();
	for (uint32 i = 0; i < count; i += 4) {
		pixel[i + 3] = 255;
	}

	FirstBitmap = new BBitmap(OriginalBitmap);
	Ratio = OriginalBitmap->Bounds().right / OriginalBitmap->Bounds().bottom;

	//on resize la fenetre sans recalculer la view a l'interieur
	DontResize = true;
	ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom); //grosseur originale
	Window()->ResizeTo(OriginalBitmap->Bounds().right, OriginalBitmap->Bounds().bottom);
	
	//on fait reaparaitre nos fenetres	
	Window()->Show();
	((Resizer*)be_app)->Option->Lock();
	((Resizer*)be_app)->Option->Show();
	((Resizer*)be_app)->Option->Option->Sync();
	((Resizer*)be_app)->Option->Unlock();
	Flush();
}
//-------------------------------------------------------------------
void MainView::BlackAndWhite() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Darker = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color CurrentColor;
	int moy;
	Window()->SetTitle("Working...");
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			CurrentColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			moy = (int)((CurrentColor.red + CurrentColor.green + CurrentColor.blue)/3);
			((rgb_color *)Darker->Bits())[row*widthSize + col].red = moy; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].green = moy; 
			((rgb_color *)Darker->Bits())[row*widthSize + col].blue = moy;
			((rgb_color *)Darker->Bits())[row*widthSize + col].alpha = CurrentColor.alpha; 
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Darker;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::SmoothScale() 
{	/*Si on fait ca, le original bitmap est resize et on peut plus 
	revenir en arriere pour la size sans Reseter...*/
	int x, y, z, cRed, cGreen, cBlue, cAlpha;
	int ox=0; int oy=0; int ow=0; int oh=0;
	if(OriginalBitmap == NULL) return;
	int OwidthSize = (int)(OriginalBitmap->BytesPerRow()/4);
	ow = (int)OriginalBitmap->Bounds().right+1;
	oh = (int)OriginalBitmap->Bounds().bottom+1;

	((Resizer*)be_app)->Option->Lock();
	int w = atoi(((Resizer*)be_app)->Option->Option->Largeur->Text());
	int h = atoi(((Resizer*)be_app)->Option->Option->Hauteur->Text());
	((Resizer*)be_app)->Option->Option->Sync();
	((Resizer*)be_app)->Option->Unlock();

	if((ow < w) || (oh < h))
		return; //smooth scaling marche seulement en shrinking.

	BBitmap* Smooth = new BBitmap(BRect(0,0, w-1, h-1), B_RGBA32, true);
	int widthSize = (int)(Smooth->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	for(y=0; y < h; y++)
		for(x=0; x < w; x++)
		{//pour chaque pixel
			z=0; cRed=0; cGreen=0; cBlue=0; cAlpha = 0;
			for(oy = (y*oh)/h; oy < ((y+1)*oh)/h; oy++)
				for(ox = (x*ow)/w; ox < ((x+1)*ow)/w; ox++)
				{
					cRed += ((rgb_color *)OriginalBitmap->Bits())[oy*OwidthSize + ox].red;
					cGreen += ((rgb_color *)OriginalBitmap->Bits())[oy*OwidthSize + ox].green;
					cBlue += ((rgb_color *)OriginalBitmap->Bits())[oy*OwidthSize + ox].blue;
					z++; //prochain pixel
				}

			((rgb_color *)Smooth->Bits())[y*widthSize + x].red = cRed/z;
			((rgb_color *)Smooth->Bits())[y*widthSize + x].green = cGreen/z;
			((rgb_color *)Smooth->Bits())[y*widthSize + x].blue = cBlue/z;
			((rgb_color *)Smooth->Bits())[y*widthSize + x].alpha = cAlpha/z;
		}

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Smooth;
	Invalidate();
}
//-------------------------------------------------------------------

//-------------------------------------------------------------------
void MainView::SmoothScale(BBitmap* origin, BBitmap* destination)
{	/*Si on fait ca, le original bitmap est resize et on peut plus
	revenir en arriere pour la size sans Reseter...*/
	int x, y, z, cRed, cGreen, cBlue, cAlpha;
	int ox=0; int oy=0; int ow=0; int oh=0;
	if(origin == NULL) return;
	int OwidthSize = (int)(origin->BytesPerRow()/4);
	ow = (int)origin->Bounds().right+1;
	oh = (int)origin->Bounds().bottom+1;
	int w = (int)destination->Bounds().right+1;
	int h = (int)destination->Bounds().bottom+1;

	if((ow < w) || (oh < h))
		return; //smooth scaling marche seulement en shrinking.

	int widthSize = (int)(destination->BytesPerRow()/4);

	for(y=0; y < h; y++)
		for(x=0; x < w; x++)
		{//pour chaque pixel
			z=0; cRed=0; cGreen=0; cBlue=0; cAlpha = 0;
			for(oy = (y*oh)/h; oy < ((y+1)*oh)/h; oy++)
				for(ox = (x*ow)/w; ox < ((x+1)*ow)/w; ox++)
				{
					cRed += ((rgb_color *)origin->Bits())[oy*OwidthSize + ox].red;
					cGreen += ((rgb_color *)origin->Bits())[oy*OwidthSize + ox].green;
					cBlue += ((rgb_color *)origin->Bits())[oy*OwidthSize + ox].blue;
					cAlpha += ((rgb_color *)origin->Bits())[oy*OwidthSize + ox].alpha;
					z++; //prochain pixel
				}
			((rgb_color *)destination->Bits())[y*widthSize + x].red = cRed/z;
			((rgb_color *)destination->Bits())[y*widthSize + x].green = cGreen/z;
			((rgb_color *)destination->Bits())[y*widthSize + x].blue = cBlue/z;
			((rgb_color *)destination->Bits())[y*widthSize + x].alpha = cAlpha/z;
		}
}
//-------------------------------------------------------------------


void MainView::Melt()
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Smoothed = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color RatioMajor, RatioMinor, RatioFinal;
	//RatioFinal.alpha = 255;

	Window()->SetTitle("Working...");
	//premiere rangee, non modifiee
	for(int RowSpecial = 0; RowSpecial < width; RowSpecial++)
		((rgb_color *)Smoothed->Bits())[RowSpecial] = 
		((rgb_color *)OriginalBitmap->Bits())[RowSpecial];

	for(int row = 1; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			RatioMajor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			RatioMinor = ((rgb_color *)OriginalBitmap->Bits())[(row-1)*widthSize + col];
			RatioFinal.red = (int)((RatioMajor.red * 0.70) + (RatioMinor.red * 0.30));
			RatioFinal.green = (int)((RatioMajor.green * 0.70) + (RatioMinor.green * 0.30));
			RatioFinal.blue = (int)((RatioMajor.blue * 0.70) + (RatioMinor.blue * 0.30));
			RatioFinal.alpha = (int)((RatioMajor.alpha * 0.70) + (RatioMinor.alpha * 0.30));
			RatioFinal.alpha = RatioMajor.alpha;
			((rgb_color *)Smoothed->Bits())[row*widthSize + col] = RatioFinal; 
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Smoothed;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::Invert() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Inverted = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color invertedColor;

	Window()->SetTitle("Working...");
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			invertedColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			((rgb_color *)Inverted->Bits())[row*widthSize + col].red = 255 - invertedColor.red; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].green = 255 - invertedColor.green; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].blue = 255 - invertedColor.blue; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].alpha = invertedColor.alpha;
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Inverted;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::Drunk()
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Barney = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	rgb_color PC; //preceding color
	rgb_color NC; //next color
	
	Window()->SetTitle("Working...");
	//premiere et derniere rangees non modifiee
	for(int RowSpecial = 0; RowSpecial < width; RowSpecial++)
		((rgb_color *)Barney->Bits())[RowSpecial] = 
		((rgb_color *)OriginalBitmap->Bits())[RowSpecial];

	for(int RowSpecial = 0; RowSpecial < width; RowSpecial++)
		((rgb_color *)Barney->Bits())[(height-1)*widthSize + RowSpecial] = 
		((rgb_color *)OriginalBitmap->Bits())[(height-1)*widthSize + RowSpecial];

	for(int row = 1; row < height-1; row++)
		for(int col = 0; col < width; col++)
		{
			PC = ((rgb_color *)OriginalBitmap->Bits())[(row-1)*widthSize + col];
			NC = ((rgb_color *)OriginalBitmap->Bits())[(row+1)*widthSize + col];
			((rgb_color *)Barney->Bits())[row*widthSize + col].red = (int)((PC.red + NC.red)/2);
			((rgb_color *)Barney->Bits())[row*widthSize + col].green = (int)((PC.green + NC.green)/2);
			((rgb_color *)Barney->Bits())[row*widthSize + col].blue = (int)((PC.blue + NC.blue)/2);
			((rgb_color *)Barney->Bits())[row*widthSize + col].alpha = (int)((PC.alpha + NC.alpha)/2);
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Barney;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::InverseRG() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Inverted = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	rgb_color invertedColor;
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			invertedColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			((rgb_color *)Inverted->Bits())[row*widthSize + col].red = invertedColor.green; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].green = invertedColor.red; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].blue = invertedColor.blue; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].alpha = invertedColor.alpha;
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Inverted;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::InverseRB() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Inverted = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	rgb_color invertedColor;
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			invertedColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			((rgb_color *)Inverted->Bits())[row*widthSize + col].red = invertedColor.blue; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].blue = invertedColor.red; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].green = invertedColor.green;
			((rgb_color *)Inverted->Bits())[row*widthSize + col].alpha = invertedColor.alpha;
		}			

	AddBitmap(OriginalBitmap);
	OriginalBitmap = Inverted;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::InverseGB() 
{
	if(OriginalBitmap == NULL) return;
	BBitmap* Inverted = new BBitmap(OriginalBitmap->Bounds(), B_RGBA32, true);

	int width = (int)OriginalBitmap->Bounds().right+1;
	int height = (int)OriginalBitmap->Bounds().bottom+1;
	int widthSize = (int)(OriginalBitmap->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	rgb_color invertedColor;
	for(int row = 0; row < height; row++)
		for(int col = 0; col < width; col++)
		{
			invertedColor = ((rgb_color *)OriginalBitmap->Bits())[row*widthSize + col];
			((rgb_color *)Inverted->Bits())[row*widthSize + col].green = invertedColor.blue; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].blue = invertedColor.green; 
			((rgb_color *)Inverted->Bits())[row*widthSize + col].red = invertedColor.red;
			((rgb_color *)Inverted->Bits())[row*widthSize + col].alpha = invertedColor.alpha;
		}			
	
	AddBitmap(OriginalBitmap);
	OriginalBitmap = Inverted;
	Invalidate();
}
//-------------------------------------------------------------------
void MainView::AddDesktopBackground()
{
	if(OriginalBitmap == NULL) return;
	BBitmap* DesktopBackground = new BBitmap(OriginalBitmap, B_RGBA32, true);

	int width = (int)DesktopBackground->Bounds().right+1;
	int height = (int)DesktopBackground->Bounds().bottom+1;
	int widthSize = (int)(DesktopBackground->BytesPerRow()/4);

	Window()->SetTitle("Working...");
	rgb_color desktopColor =  BScreen(Window()).DesktopColor();

	uint8* pixel = (uint8*)DesktopBackground->Bits();
	uint32 numPixels = DesktopBackground->BitsLength() / 4;

	for (uint32 i = 0; i < numPixels; i++) {
		if (pixel[3] < 255) {
			uint8 alpha = 255 - pixel[3];
			blend_colors(pixel, desktopColor.red, desktopColor.green, desktopColor.blue, alpha);
			pixel[3] = 255;
		}
		pixel += 4;
	}

	AddBitmap(OriginalBitmap);
	OriginalBitmap = DesktopBackground;
	Invalidate();
}
//-------------------------------------------------------------------
