/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#include "MainView.h"
#include "RefFilters.h"
#include "WindowTitleGuard.h"
#include "main.h"

#include <Application.h>
#include <BitmapStream.h>
#include <Catalog.h>
#include <Directory.h>
#include <File.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Screen.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>

#include <stdlib.h>
#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainView"


#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) > (b) ? (a) : (b))

const int FACTOR = 2;


MainView::MainView()
	:
	BView(BRect(0, 0, 400, 100), "Image", B_FOLLOW_NONE, B_WILL_DRAW)
{
	SetViewColor(128, 128, 128);
	fOriginalBitmap = NULL;
	fModifiedBitmap = NULL;
	fOffscreenBitmap = NULL; // to avoid flickering
	SetViewColor(B_TRANSPARENT_32_BIT);
	fDontResize = false;
	fKeepRatio = true;
	fOriginalBitmap = NULL;

	// no output selected
	fDragging = false;
	all_translators = NULL;
	fCurrentTranslator = -1;
	fCurrentOutput = -1;
	fClipPoint1 = BPoint(-1, -1);
	fClipPoint2 = BPoint(-1, -1);

	fOpenPanel
		= new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, true, NULL, new ImageFilter());
}


MainView::~MainView()
{
	if (fOriginalBitmap != NULL) {
		delete fOriginalBitmap;
		delete fFirstBitmap;
	}
	delete fOpenPanel;
}


BRect
MainView::GetRegion()
{ // Return the region to be selected between clipping 1 and 2
	return BRect(min(fClipPoint1.x, fClipPoint2.x), min(fClipPoint1.y, fClipPoint2.y),
		max(fClipPoint1.x, fClipPoint2.x), max(fClipPoint1.y, fClipPoint2.y));
}


bool
MainView::GetImage(const char* path)
{
	if (fOriginalBitmap != NULL) {
		delete fOriginalBitmap;
		delete fFirstBitmap;
	}

	BBitmap* AnyImageFormat = BTranslationUtils::GetBitmap(path);
	if (AnyImageFormat == NULL)
		return false;

	// Convert the bitmap to RGB32 for the effects to work
	BRect rect = AnyImageFormat->Bounds();
	fOriginalBitmap = new BBitmap(rect, B_RGB32, true);
	fOffscreenView = new BView(rect, "", B_FOLLOW_NONE, (uint32)NULL);
	fOriginalBitmap->AddChild(fOffscreenView);
	fOriginalBitmap->Lock();
	fOffscreenView->DrawBitmap(AnyImageFormat, rect);
	fOffscreenView->Sync();
	fOriginalBitmap->Unlock();
	fOriginalBitmap->RemoveChild(fOffscreenView);
	delete AnyImageFormat;

	fFirstBitmap = new BBitmap(fOriginalBitmap);
	fRatio = fOriginalBitmap->Bounds().right / fOriginalBitmap->Bounds().bottom;

	// resize the window without recalculating the cintaining view
	fDontResize = true;
	ResizeTo(fOriginalBitmap->Bounds().right,
		fOriginalBitmap->Bounds().bottom); // grosseur originale
	Window()->ResizeTo(fOriginalBitmap->Bounds().right, fOriginalBitmap->Bounds().bottom);
	Flush(); // flush les vieux undo...
	BString windowTitle(path);
	windowTitle.Remove(0, windowTitle.FindLast("/") + 1);
	windowTitle.Prepend("TAR: ");

	Window()->SetTitle(windowTitle);
	return true;
}


void
MainView::ResizeImage()
{
	// Cancel clipping region if there is one
	fClipPoint1 = BPoint(-1, -1);
	fClipPoint2 = BPoint(-1, -1);

	BRect winRect = Window()->Bounds();

	if (fOriginalBitmap == NULL) { // When the window gets resized, the view is enlarged to take all
								   // the available space
		ResizeTo(winRect.right, winRect.bottom);
		Invalidate();
	}

	else if (fDontResize) { // cas special pour le drop pour pas caller le resize
		fDontResize = false;
		Invalidate();
	}

	else if (fKeepRatio) { // resize the view while keeping the ratio intact
		// Adjustment as a function of width
		ResizeTo(winRect.bottom * fRatio, winRect.bottom);
		Window()->ResizeTo(winRect.bottom * fRatio, winRect.bottom);
		Invalidate();
	}

	else {
		ResizeTo(winRect.right, winRect.bottom);
		Invalidate();
	}

	if (((Resizer*)be_app)->fMainWin->fDontUpdate) { // The resize was caused by a manual entry in
													 // the option window, do not react
		((Resizer*)be_app)->fMainWin->fDontUpdate = false;
		return;
	}

	BMessage* message = new BMessage(TEXT_WIDTH);
	message->AddInt16("NewWidth", Bounds().right + 1);
	((Resizer*)be_app)->fOptionWin->PostMessage(message);

	BMessage* message2 = new BMessage(TEXT_HEIGHT);
	message2->AddInt16("NewHeight", Bounds().bottom + 1);
	((Resizer*)be_app)->fOptionWin->PostMessage(message2);
}


void
MainView::ResizeImage(int width, int height)
{ // The user entered values ​​manually, we treat the input of the user
	if (fOriginalBitmap == NULL) {
		int w = width;
		int h = height;
		if (w == -1)
			w = (int)Bounds().right - 1; // We keep the same value
		else if (h == -1)
			h = (int)Bounds().bottom - 1; // We keep the same value
		Window()->ResizeTo(w, h); // resize la fenetre (on veut juste pas planter)
	}

	else if (fKeepRatio) { // resize the view as the user wants
		int W = 0;
		int H = 0;

		if (width == -1) {
			H = height - 1;
			W = (int)(fRatio * H);

			BMessage* message = new BMessage(TEXT_WIDTH);
			message->AddInt16("NewWidth", W + 1);
			((Resizer*)be_app)->fOptionWin->PostMessage(message);
		}

		else if (height == -1) {
			W = width - 1;
			H = (int)(W / fRatio);

			BMessage* message2 = new BMessage(TEXT_HEIGHT);
			message2->AddInt16("NewHeight", H + 1);
			((Resizer*)be_app)->fOptionWin->PostMessage(message2);
		}

		fDontResize = true; // don't resize the view when resizing the window
		ResizeTo(W, H); // resize the image
		Window()->ResizeTo(W, H); // fit the window to the image
		Invalidate();
	}

	else { // don't keep the aspect ration, just use the provided values
		int W = 0;
		int H = 0;

		if (width == -1) {
			H = height - 1;
			W = (int)Bounds().right;
		}

		else if (height == -1) {
			W = width - 1;
			H = (int)Bounds().bottom;
		}

		fDontResize = true; // don't resize the view when resizing the window
		ResizeTo(W, H); // resize the image
		Window()->ResizeTo(W, H); // fit the window to the image
		Invalidate();
	}
}


void
MainView::Draw(BRect rect)
{
	BRect B = Bounds();
	if (fOriginalBitmap == NULL) {
		BRect bounds = B;
		const pattern stripePattern = {0xcc, 0x66, 0x33, 0x99, 0xcc, 0x66, 0x33, 0x99};

		font_height fontHeight;
		GetFontHeight(&fontHeight);
		const char* stringMessage = B_TRANSLATE_COMMENT(
			"Drag and drop to open/save an image", "Short as possible");
		const char* text = stringMessage;

		float width = StringWidth(text);

		BString truncated;
		if (width - 10 > bounds.Width()) {
			truncated = stringMessage;
			TruncateString(&truncated, B_TRUNCATE_END, bounds.Width());
			text = truncated.String();
			width = StringWidth(text);
		}

		float y
			= (bounds.top + bounds.bottom - ceilf(fontHeight.ascent) - ceilf(fontHeight.descent))
				/ 2.0
			+ ceilf(fontHeight.ascent);
		float x = (bounds.Width() - width) / 2.0;

		SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(bounds);

		SetDrawingMode(B_OP_ALPHA);
		SetLowColor(0, 0, 0, 0);
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));

		FillRect(bounds, B_SOLID_LOW);
		StrokeRect(bounds);
		FillRect(bounds.InsetBySelf(3, 3), stripePattern);

		BRect labelBackground(x - 5, y - ceilf(fontHeight.ascent), x + width + 5,
			y + ceilf(fontHeight.descent));
		SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(labelBackground, B_SOLID_LOW);
		SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
		DrawString(text, BPoint(x, y));

		BView::Draw(rect);
		return;
	}
	delete fOffscreenBitmap;
	fOffscreenBitmap = new BBitmap(B, B_RGB32, true);
	fOffscreenView = new BView(B, "", B_FOLLOW_NONE, (uint32)NULL);
	fOffscreenBitmap->AddChild(fOffscreenView);

	fOffscreenBitmap->Lock(); // protect the offscreen bitmap against changes

	((Resizer*)be_app)->fOptionWin->Lock();
	bool smoothScaling = ((Resizer*)be_app)->fOptionWin->fSmoothBox->Value() != 0;
	((Resizer*)be_app)->fOptionWin->Sync();
	((Resizer*)be_app)->fOptionWin->Unlock();

	if (!smoothScaling
		|| (fOriginalBitmap->Bounds().Width() <= B.Width()
			&& fOriginalBitmap->Bounds().Height() <= B.Height())) {
		fOffscreenView->DrawBitmap(fOriginalBitmap, fOriginalBitmap->Bounds(), B,
			smoothScaling ? B_FILTER_BITMAP_BILINEAR : 0);
	} else
		SmoothScale(fOriginalBitmap, fOffscreenBitmap);

	// Draw the invalidated portion of the offscreen bitmap into the onscreen view
	fOffscreenView->Sync(); // Synchronise the offscreen view
	DrawBitmap(fOffscreenBitmap); // Copy bitmap to screen
	fOffscreenBitmap->Unlock();
	fOffscreenBitmap->RemoveChild(fOffscreenView);

	if (fClipPoint1 != BPoint(-1, -1) && fClipPoint2 != BPoint(-1, -1)) {
		SetHighColor(180, 0, 180, 255);
		SetLowColor(255, 255, 255);
		StrokeRect(GetRegion(), B_MIXED_COLORS);
		SetHighColor(255, 255, 255, 255);
	}

	delete fOffscreenView;
	// delete fOffscreenBitmap;
}


void
MainView::MessageReceived(BMessage* message)
{
	entry_ref ref;
	BPath path;

	switch (message->what) {
		case B_SIMPLE_DATA:
		{ // Look for a ref in the message
			if (message->FindRef("refs", &ref)
				== B_OK) { // Call SetText() to change the string in the view
				BEntry Entry(&ref);
				Entry.GetPath(&path);
				((Resizer*)be_app)->fOptionWin->Lock();
				((Resizer*)be_app)->fOptionWin->fFileName->SetText(ref.name);
				((Resizer*)be_app)->fOptionWin->Sync();
				((Resizer*)be_app)->fOptionWin->Unlock();
				GetImage(path.Path());
				Window()->PostMessage(CHANGE_FILE);
			}

			else {
				BView::MessageReceived(message);
			}
			break;
		}

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
MainView::ToggleRatio()
{
	fKeepRatio = !fKeepRatio;
	ResizeImage();
}


void
MainView::ResetImage()
{
	if (fOriginalBitmap == NULL)
		return;

	delete fOriginalBitmap;
	fOriginalBitmap = new BBitmap(fFirstBitmap);
	fRatio = fOriginalBitmap->Bounds().right / fOriginalBitmap->Bounds().bottom;
	fKeepRatio = true;
	fDontResize = true; // pour pas caller de resize de la view en partant
	ResizeTo(fOriginalBitmap->Bounds().right, fOriginalBitmap->Bounds().bottom);
	Window()->ResizeTo(fOriginalBitmap->Bounds().right, fOriginalBitmap->Bounds().bottom);
	Invalidate();
}


void
MainView::MouseDown(BPoint where)
{
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if (buttons == B_SECONDARY_MOUSE_BUTTON) { // select a region
		fClipPoint1 = where;
		fDragging = true;

		BMessage* Message = new BMessage(CLIP1);
		Message->AddPoint("Clip1", fClipPoint1);
		((Resizer*)be_app)->fMouseWin->PostMessage(Message);

		return;
	}

	// Left click on empty window
	else if (buttons == B_PRIMARY_MOUSE_BUTTON
		&& fOriginalBitmap == NULL) {
		fOpenPanel->Show();
	}

	else if (!BRect(GetRegion()).Contains(where)) {
		fClipPoint1 = BPoint(-1, -1); // unselect when clicked outside region
		fClipPoint2 = BPoint(-1, -1);
		Invalidate();

		BMessage* Message = new BMessage(CLIP0);
		((Resizer*)be_app)->fMouseWin->PostMessage(Message);
	}

	// We start to drag the image or part of the image
	if (fOriginalBitmap == NULL || fCurrentTranslator == -1)
		return;

	BPoint P;
	uint32 mod;
	while (true) {
		GetMouse(&P, &mod);
		// aucun bouton enfonce
		if (!mod) {
			Window()->Activate(true);
			return;
		}

		if (P != where)
			break;
		snooze(40000);
	}

	BMessage Message(B_SIMPLE_DATA);
	Message.AddString("be:types", B_FILE_MIME_TYPE);

	const translation_format* FormatOut;
	int32 FormatOutCount;
	BTranslatorRoster::Default()->GetOutputFormats(all_translators[fCurrentTranslator], &FormatOut,
		&FormatOutCount);
	Message.AddString("be:types", FormatOut[fCurrentOutput].MIME);
	Message.AddString("be:filetypes", FormatOut[fCurrentOutput].MIME);
	Message.AddInt32("be:actions", B_COPY_TARGET);

	((Resizer*)be_app)->fOptionWin->Lock();
	Message.AddString("be:clip_name",
		((Resizer*)be_app)->fOptionWin->fFileName->Text());
	((Resizer*)be_app)->fOptionWin->Sync();
	((Resizer*)be_app)->fOptionWin->Unlock();

	DragMessage(&Message, BRect(where.x - 10, where.y - 10, where.x + 10, where.y + 10), Window());
}


void
MainView::MouseUp(BPoint where)
{
	// we were selecting a region
	if (fDragging) {
		fDragging = false;
		BPoint P;
		uint32 mod;
		GetMouse(&P, &mod);
		fClipPoint2 = P;
		Invalidate();

		BMessage* Message = new BMessage(CLIP2);
		Message->AddPoint("Clip2", fClipPoint2);
		((Resizer*)be_app)->fMouseWin->PostMessage(Message);
	}
}


void
MainView::MouseMoved(BPoint point, uint32 transit, const BMessage* message)
{
	BMessage* Message = new BMessage(MOVEDPOINT);
	Message->AddPoint("MovedPoint", point);
	((Resizer*)be_app)->fMouseWin->PostMessage(Message);

	if (fDragging) {
		if (fClipPoint2 != point) {
			fClipPoint2 = point;
			Invalidate();

			BMessage* Message = new BMessage(CLIP2);
			Message->AddPoint("Clip2", fClipPoint2);
			((Resizer*)be_app)->fMouseWin->PostMessage(Message);
		}
	}
}


void
MainView::Copy(BMessage* request)
{
	if (fClipPoint1 != BPoint(-1, -1) && fClipPoint2 != BPoint(-1, -1)) { // copy the selected
																		  // region
		BRect Source = GetRegion();
		BRect Destination = BRect(0, 0, Source.right - Source.left, Source.bottom - Source.top);

		BBitmap* temp = new BBitmap(Bounds(), B_RGB32, true);
		BView* OffView = new BView(Bounds(), "", B_FOLLOW_NONE, (uint32)NULL);
		temp->AddChild(OffView);
		temp->Lock();
		OffView->DrawBitmap(fOffscreenBitmap, Bounds());
		OffView->Sync();
		temp->Unlock();
		temp->RemoveChild(OffView);
		delete OffView;

		// Apply clipping
		fModifiedBitmap = new BBitmap(Destination, B_RGB32, true);
		OffView = new BView(Destination, "", B_FOLLOW_NONE, (uint32)NULL);
		fModifiedBitmap->AddChild(OffView);
		fModifiedBitmap->Lock();
		OffView->DrawBitmap(temp, Source, Destination);
		OffView->Sync();
		fModifiedBitmap->Unlock();
		fModifiedBitmap->RemoveChild(OffView);
		delete OffView;
		delete temp;
	}

	else { // copy whole image
		fModifiedBitmap = new BBitmap(Bounds(), B_RGB32, true);
		BView* OffView = new BView(Bounds(), "", B_FOLLOW_NONE, (uint32)NULL);
		fModifiedBitmap->AddChild(OffView);
		fModifiedBitmap->Lock();
		OffView->DrawBitmap(fOffscreenBitmap, Bounds());
		OffView->Sync();
		fModifiedBitmap->Unlock();
		fModifiedBitmap->RemoveChild(OffView);
		delete OffView;
	}

	const char* type = NULL;
	if (!request->FindString("be:types", &type)) {
		BBitmapStream stream(fModifiedBitmap);
		const translation_format* FormatOut;
		int32 FormatOutCount;
		BTranslatorRoster::Default()->GetOutputFormats(all_translators[fCurrentTranslator],
			&FormatOut, &FormatOutCount);

		if (!strcasecmp(type, B_FILE_MIME_TYPE)) {
			const char* name;
			entry_ref dir;
			if (!request->FindString("be:filetypes", &type) && !request->FindString("name", &name)
				&& !request->FindRef("directory", &dir)) { //	write file
				BDirectory d(&dir);
				BFile f(&d, name, O_RDWR | O_TRUNC);
				BTranslatorRoster::Default()->Translate(all_translators[fCurrentTranslator],
					&stream, NULL, &f, FormatOut[fCurrentOutput].type);
				BNodeInfo ni(&f);
				ni.SetType(FormatOut[fCurrentOutput].MIME);
			}
		}

		else {
			BMessage msg(B_MIME_DATA);
			BMallocIO f;
			BTranslatorRoster::Default()->Translate(all_translators[fCurrentTranslator], &stream,
				NULL, &f, FormatOut[fCurrentOutput].type);
			msg.AddData(FormatOut[fCurrentOutput].MIME, B_MIME_TYPE, f.Buffer(), f.BufferLength());
			request->SendReply(&msg);
		}
		stream.DetachBitmap(&fModifiedBitmap);
	}
	delete fModifiedBitmap;
}

/*===================================================================
//UNDO
===================================================================*/
void
MainView::AddBitmap(BBitmap* bitmap)
{
	fFile.push_back(bitmap);
	if (fFile.size() > 8) { // on efface le plus vieux
		delete fFile.front();
		fFile.pop_front();
	}

	BMessage* message = new BMessage(UNDO_OK);
	((Resizer*)be_app)->fOptionWin->PostMessage(message);
}


void
MainView::Undo()
{
	if (fFile.empty())
		return;
	delete fOriginalBitmap;
	fOriginalBitmap = fFile.back();
	fFile.pop_back(); // remove from list
	Invalidate();
	if (fFile.empty()) {
		BMessage* message = new BMessage(UNDO_NOT_OK);
		((Resizer*)be_app)->fOptionWin->PostMessage(message);
	}
}


void
MainView::Flush()
{
	while (!fFile.empty()) {
		delete fFile.back();
		fFile.pop_back();
	}

	BMessage* message = new BMessage(UNDO_NOT_OK);
	((Resizer*)be_app)->fOptionWin->PostMessage(message);
}

/*===================================================================
//EFFECTS
===================================================================*/
void
MainView::RotateImage()
{ // create a new image, 90° rotated
	if (fOriginalBitmap == NULL)
		return;
	BRect coord = BRect(0, 0, fOriginalBitmap->Bounds().bottom, fOriginalBitmap->Bounds().right);
	BBitmap* Spin = new BBitmap(coord, B_RGB32, true);

	int widthO = (int)fOriginalBitmap->Bounds().right + 1;
	int heightO = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthD = (int)Spin->Bounds().right + 1;

	for (int row = 0; row < heightO; row++) {
		for (int col = 0; col < widthO; col++) {
			((rgb_color*)Spin->Bits())[(col * widthD + row)]
				= ((rgb_color*)fOriginalBitmap->Bits())[row * widthO + widthO - col];
		}
	}

	BRect temp = fOriginalBitmap->Bounds();
	delete fOriginalBitmap;
	fOriginalBitmap = Spin;
	fRatio = fOriginalBitmap->Bounds().right / fOriginalBitmap->Bounds().bottom;
	ResizeImage(temp.bottom + 1, -1);
	ResizeImage(-1, temp.right + 1);
	Flush();
}


void
MainView::Flip(bool horizontal)
{
	if (fOriginalBitmap == NULL)
		return;
	fClipPoint1 = BPoint(-1, -1);
	fClipPoint2 = BPoint(-1, -1);
	BBitmap* Flip = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	if (horizontal) {
		for (int row = 0; row < height; row++) {
			for (int col = 0; col < width; col++) {
				((rgb_color*)Flip->Bits())[((height - 1 - row) * widthSize + col)]
					= ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			}
		}
	}

	else { // vertical
		for (int row = 0; row < height; row++) {
			for (int col = 0; col < width; col++) {
				((rgb_color*)Flip->Bits())[(row * widthSize + width - 1 - col)]
					= ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			}
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Flip;
	Invalidate();
}


void
MainView::Dark()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Darker = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color CurrentColor;
	int r, g, b;
	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			CurrentColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			r = CurrentColor.red - FACTOR;
			g = CurrentColor.green - FACTOR;
			b = CurrentColor.blue - FACTOR;
			if (r < 0)
				r = 0;
			if (g < 0)
				g = 0;
			if (b < 0)
				b = 0;
			((rgb_color*)Darker->Bits())[row * widthSize + col].red = r;
			((rgb_color*)Darker->Bits())[row * widthSize + col].green = g;
			((rgb_color*)Darker->Bits())[row * widthSize + col].blue = b;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Darker;
	Invalidate();
}


void
MainView::Light()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Darker = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color CurrentColor;
	int r, g, b;
	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			CurrentColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			r = CurrentColor.red + FACTOR;
			g = CurrentColor.green + FACTOR;
			b = CurrentColor.blue + FACTOR;
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;
			((rgb_color*)Darker->Bits())[row * widthSize + col].red = r;
			((rgb_color*)Darker->Bits())[row * widthSize + col].green = g;
			((rgb_color*)Darker->Bits())[row * widthSize + col].blue = b;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Darker;
	Invalidate();
}


void
MainView::Blur()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Blured = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));

	rgb_color CC; // current color
	rgb_color T; // top color
	rgb_color L; // left color
	rgb_color R; // right color
	rgb_color D; // down color
	rgb_color TR; // top right color
	rgb_color TL; // top left color
	rgb_color BL; // down left color
	rgb_color BR; // down right color
	rgb_color final; // final color
	rgb_color empty;
	empty.red = 0;
	empty.green = 0;
	empty.blue = 0;
	empty.alpha = 255; // no color
	int contour;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			contour = 0;
			CC = ((rgb_color*)fOriginalBitmap->Bits())[(row)*widthSize + col];

			if (row > 0) {
				T = ((rgb_color*)fOriginalBitmap->Bits())[(row - 1) * widthSize + col];
				contour++;
			} else
				T = empty;

			if (col > 0) {
				L = ((rgb_color*)fOriginalBitmap->Bits())[(row)*widthSize + col - 1];
				contour++;
			} else
				L = empty;

			if (col < width - 1) {
				R = ((rgb_color*)fOriginalBitmap->Bits())[(row)*widthSize + col + 1];
				contour++;
			} else
				R = empty;

			if (row < height - 1) {
				D = ((rgb_color*)fOriginalBitmap->Bits())[(row + 1) * widthSize + col];
				contour++;
			} else
				D = empty;

			if (row > 0 && col > 0) {
				TL = ((rgb_color*)fOriginalBitmap->Bits())[(row - 1) * widthSize + col - 1];
				contour++;
			} else
				TL = empty;

			if (row > 0 && col < width - 1) {
				TR = ((rgb_color*)fOriginalBitmap->Bits())[(row - 1) * widthSize + col + 1];
				contour++;
			} else
				TR = empty;

			if (row < height - 1 && col > 0) {
				BL = ((rgb_color*)fOriginalBitmap->Bits())[(row + 1) * widthSize + col - 1];
				contour++;
			} else
				BL = empty;

			if (row < height - 1 && col < width - 1) {
				BR = ((rgb_color*)fOriginalBitmap->Bits())[(row + 1) * widthSize + col + 1];
				contour++;
			} else
				BR = empty;

			final.red = (int)(((L.red + R.red + T.red + D.red + TL.red + TR.red + BL.red + BR.red)
								  / (4 * contour))
				+ (CC.red * 0.75));
			final.green = (int)(((L.green + R.green + T.green + D.green + TL.green + TR.green
									 + BL.green + BR.green)
									/ (4 * contour))
				+ (CC.green * 0.75));
			final.blue
				= (int)(((L.blue + R.blue + T.blue + D.blue + TL.blue + TR.blue + BL.blue + BR.blue)
							/ (4 * contour))
					+ (CC.blue * 0.75));

			((rgb_color*)Blured->Bits())[row * widthSize + col] = final;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Blured;
	Invalidate();
}


void
MainView::GrabScreen()
{
	((Resizer*)be_app)->fOptionWin->Lock();
	((Resizer*)be_app)->fOptionWin->fFileName->SetText("screenshot");
	((Resizer*)be_app)->fOptionWin->Hide();
	((Resizer*)be_app)->fOptionWin->Sync();
	((Resizer*)be_app)->fOptionWin->Unlock();
	Window()->Hide();

	if (fOriginalBitmap != NULL) {
		delete fOriginalBitmap;
		delete fFirstBitmap;
	}

	BBitmap* AnyImageFormat;
	BScreen* TheScreen = new BScreen();
	snooze(500000);
	TheScreen->GetBitmap(&AnyImageFormat);
	delete TheScreen;

	// convert to RGB32 to make the effects work
	BRect rect = AnyImageFormat->Bounds();
	fOriginalBitmap = new BBitmap(rect, B_RGB32, true);
	fOffscreenView = new BView(rect, "", B_FOLLOW_NONE, (uint32)NULL);
	fOriginalBitmap->AddChild(fOffscreenView);
	fOriginalBitmap->Lock();
	fOffscreenView->DrawBitmap(AnyImageFormat, rect);
	fOffscreenView->Sync();
	fOriginalBitmap->Unlock();
	fOriginalBitmap->RemoveChild(fOffscreenView);

	delete AnyImageFormat;
	fFirstBitmap = new BBitmap(fOriginalBitmap);
	fRatio = fOriginalBitmap->Bounds().right / fOriginalBitmap->Bounds().bottom;

	// on resize la fenetre sans recalculer la view a l'interieur
	fDontResize = true;
	ResizeTo(fOriginalBitmap->Bounds().right,
		fOriginalBitmap->Bounds().bottom); // grosseur originale
	Window()->ResizeTo(fOriginalBitmap->Bounds().right, fOriginalBitmap->Bounds().bottom);

	// on fait reaparaitre nos fenetres
	Window()->Show();
	((Resizer*)be_app)->fOptionWin->Lock();
	((Resizer*)be_app)->fOptionWin->Show();
	((Resizer*)be_app)->fOptionWin->Sync();
	((Resizer*)be_app)->fOptionWin->Unlock();
	Flush();
}


void
MainView::BlackAndWhite()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Darker = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color CurrentColor;
	int moy;
	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			CurrentColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			moy = (int)((CurrentColor.red + CurrentColor.green + CurrentColor.blue) / 3);
			((rgb_color*)Darker->Bits())[row * widthSize + col].red = moy;
			((rgb_color*)Darker->Bits())[row * widthSize + col].green = moy;
			((rgb_color*)Darker->Bits())[row * widthSize + col].blue = moy;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Darker;
	Invalidate();
}


void
MainView::SmoothScale()
{ /*Si on fait ca, le original bitmap est resize et on peut plus
  revenir en arriere pour la size sans Reseter...*/
	int x, y, z, cRed, cGreen, cBlue;
	int ox = 0;
	int oy = 0;
	int ow = 0;
	int oh = 0;
	if (fOriginalBitmap == NULL)
		return;
	int OwidthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);
	ow = (int)fOriginalBitmap->Bounds().right + 1;
	oh = (int)fOriginalBitmap->Bounds().bottom + 1;

	((Resizer*)be_app)->fOptionWin->Lock();
	int w = atoi(((Resizer*)be_app)->fOptionWin->fWidthTextbox->Text());
	int h = atoi(((Resizer*)be_app)->fOptionWin->fHeightTextbox->Text());
	((Resizer*)be_app)->fOptionWin->Sync();
	((Resizer*)be_app)->fOptionWin->Unlock();

	if ((ow < w) || (oh < h))
		return; // smooth scaling only works when shrinking.

	BBitmap* Smooth = new BBitmap(BRect(0, 0, w - 1, h - 1), B_RGB32, true);
	int widthSize = (int)(Smooth->BytesPerRow() / 4);

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) { // for each pixel
			z = 0;
			cRed = 0;
			cGreen = 0;
			cBlue = 0;
			for (oy = (y * oh) / h; oy < ((y + 1) * oh) / h; oy++) {
				for (ox = (x * ow) / w; ox < ((x + 1) * ow) / w; ox++) {
					cRed += ((rgb_color*)fOriginalBitmap->Bits())[oy * OwidthSize + ox].red;
					cGreen += ((rgb_color*)fOriginalBitmap->Bits())[oy * OwidthSize + ox].green;
					cBlue += ((rgb_color*)fOriginalBitmap->Bits())[oy * OwidthSize + ox].blue;
					z++; // next pixel
				}
			}

			((rgb_color*)Smooth->Bits())[y * widthSize + x].red = cRed / z;
			((rgb_color*)Smooth->Bits())[y * widthSize + x].green = cGreen / z;
			((rgb_color*)Smooth->Bits())[y * widthSize + x].blue = cBlue / z;
			((rgb_color*)Smooth->Bits())[y * widthSize + x].alpha = 255;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Smooth;
	Invalidate();
}


void
MainView::SmoothScale(BBitmap* origin, BBitmap* destination)
{ /*Si on fait ca, le original bitmap est resize et on peut plus
  revenir en arriere pour la size sans Reseter...*/
	int x, y, z, cRed, cGreen, cBlue;
	int ox = 0;
	int oy = 0;
	int ow = 0;
	int oh = 0;
	if (origin == NULL)
		return;
	int OwidthSize = (int)(origin->BytesPerRow() / 4);
	ow = (int)origin->Bounds().right + 1;
	oh = (int)origin->Bounds().bottom + 1;
	int w = (int)destination->Bounds().right + 1;
	int h = (int)destination->Bounds().bottom + 1;

	if ((ow < w) || (oh < h))
		return; // smooth scaling only works when shrinking.

	int widthSize = (int)(destination->BytesPerRow() / 4);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) { // for each pixel
			z = 0;
			cRed = 0;
			cGreen = 0;
			cBlue = 0;
			for (oy = (y * oh) / h; oy < ((y + 1) * oh) / h; oy++) {
				for (ox = (x * ow) / w; ox < ((x + 1) * ow) / w; ox++) {
					cRed += ((rgb_color*)origin->Bits())[oy * OwidthSize + ox].red;
					cGreen += ((rgb_color*)origin->Bits())[oy * OwidthSize + ox].green;
					cBlue += ((rgb_color*)origin->Bits())[oy * OwidthSize + ox].blue;
					z++; // next pixel
				}
			}
			((rgb_color*)destination->Bits())[y * widthSize + x].red = cRed / z;
			((rgb_color*)destination->Bits())[y * widthSize + x].green = cGreen / z;
			((rgb_color*)destination->Bits())[y * widthSize + x].blue = cBlue / z;
			((rgb_color*)destination->Bits())[y * widthSize + x].alpha = 255;
		}
	}
}


void
MainView::Melt()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Smoothed = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color RatioMajor, RatioMinor, RatioFinal;
	RatioFinal.alpha = 255;

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	// premiere rangee, non modifiee
	for (int RowSpecial = 0; RowSpecial < width; RowSpecial++) {
		((rgb_color*)Smoothed->Bits())[RowSpecial]
			= ((rgb_color*)fOriginalBitmap->Bits())[RowSpecial];
	}

	for (int row = 1; row < height; row++) {
		for (int col = 0; col < width; col++) {
			RatioMajor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			RatioMinor = ((rgb_color*)fOriginalBitmap->Bits())[(row - 1) * widthSize + col];
			RatioFinal.red = (int)((RatioMajor.red * 0.70) + (RatioMinor.red * 0.30));
			RatioFinal.green = (int)((RatioMajor.green * 0.70) + (RatioMinor.green * 0.30));
			RatioFinal.blue = (int)((RatioMajor.blue * 0.70) + (RatioMinor.blue * 0.30));
			((rgb_color*)Smoothed->Bits())[row * widthSize + col] = RatioFinal;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Smoothed;
	Invalidate();
}


void
MainView::Invert()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Inverted = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color invertedColor;

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			invertedColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			((rgb_color*)Inverted->Bits())[row * widthSize + col].red = 255 - invertedColor.red;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].green = 255 - invertedColor.green;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].blue = 255 - invertedColor.blue;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Inverted;
	Invalidate();
}


void
MainView::Drunk()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Barney = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	rgb_color PC; // preceding color
	rgb_color NC; // next color

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	// premiere et derniere rangees non modifiee
	for (int RowSpecial = 0; RowSpecial < width; RowSpecial++) {
		((rgb_color*)Barney->Bits())[RowSpecial]
			= ((rgb_color*)fOriginalBitmap->Bits())[RowSpecial];
	}

	for (int RowSpecial = 0; RowSpecial < width; RowSpecial++) {
		((rgb_color*)Barney->Bits())[(height - 1) * widthSize + RowSpecial]
			= ((rgb_color*)fOriginalBitmap->Bits())[(height - 1) * widthSize + RowSpecial];
	}

	for (int row = 1; row < height - 1; row++) {
		for (int col = 0; col < width; col++) {
			PC = ((rgb_color*)fOriginalBitmap->Bits())[(row - 1) * widthSize + col];
			NC = ((rgb_color*)fOriginalBitmap->Bits())[(row + 1) * widthSize + col];
			((rgb_color*)Barney->Bits())[row * widthSize + col].red = (int)((PC.red + NC.red) / 2);
			((rgb_color*)Barney->Bits())[row * widthSize + col].green
				= (int)((PC.green + NC.green) / 2);
			((rgb_color*)Barney->Bits())[row * widthSize + col].blue
				= (int)((PC.blue + NC.blue) / 2);
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Barney;
	Invalidate();
}


void
MainView::InverseRG()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Inverted = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	rgb_color invertedColor;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			invertedColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			((rgb_color*)Inverted->Bits())[row * widthSize + col].red = invertedColor.green;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].green = invertedColor.red;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].blue = invertedColor.blue;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Inverted;
	Invalidate();
}


void
MainView::InverseRB()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Inverted = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	rgb_color invertedColor;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			invertedColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			((rgb_color*)Inverted->Bits())[row * widthSize + col].red = invertedColor.blue;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].blue = invertedColor.red;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].green = invertedColor.green;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Inverted;
	Invalidate();
}


void
MainView::InverseGB()
{
	if (fOriginalBitmap == NULL)
		return;
	BBitmap* Inverted = new BBitmap(fOriginalBitmap->Bounds(), B_RGB32, true);

	int width = (int)fOriginalBitmap->Bounds().right + 1;
	int height = (int)fOriginalBitmap->Bounds().bottom + 1;
	int widthSize = (int)(fOriginalBitmap->BytesPerRow() / 4);

	WindowTitleGuard guard(Window(), B_TRANSLATE("Working" B_UTF8_ELLIPSIS));
	rgb_color invertedColor;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			invertedColor = ((rgb_color*)fOriginalBitmap->Bits())[row * widthSize + col];
			((rgb_color*)Inverted->Bits())[row * widthSize + col].green = invertedColor.blue;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].blue = invertedColor.green;
			((rgb_color*)Inverted->Bits())[row * widthSize + col].red = invertedColor.red;
		}
	}

	AddBitmap(fOriginalBitmap);
	fOriginalBitmap = Inverted;
	Invalidate();
}


void
MainView::ClearImage()
{
	fOriginalBitmap = NULL;
	Window()->ResizeTo(400, 100);
	Window()->SetTitle(B_TRANSLATE("TAR: (No image)"));
	Invalidate();
}
