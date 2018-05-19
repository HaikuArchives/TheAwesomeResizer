#include "OptionView.h"
#include <String.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include <TranslatorFormats.h>
#include "main.h"
//----------------------------------------------------------------------
OptionView::OptionView() 
	: BView("optionView", B_WILL_DRAW)
{
	SavedW = 640; SavedH = 480; CurrentEffect = -1;
	
	Largeur = new BTextControl("Width","Width:", "", NULL);
	Largeur->SetModificationMessage(new BMessage(MOD_WIDTH)); 

	Hauteur = new BTextControl("Height", "Height:", "", NULL);
	Hauteur->SetModificationMessage(new BMessage(MOD_HEIGHT)); 

	SaveW = new BButton("SaveW", "Save", new BMessage(SAVEW));
	LoadW = new BButton("LoadW", "Load", new BMessage(LOADW));
	SaveH = new BButton("SaveH", "Save", new BMessage(SAVEH));
	LoadH = new BButton("LoadH", "Load", new BMessage(LOADH));

	CheckBox = new BCheckBox("Ratio", "Aspect Ratio", new BMessage(RATIO));
	CheckBox->SetValue(B_CONTROL_ON);

	FileName = new BTextControl("Filename","File:", "", NULL);

	//Popup qui affiche tous les translators disponibles
	Popup = new BPopUpMenu("Choose");

 	//on barre l'autre fenetre le temps de faire ca
 	((Resizer*)be_app)->Fenetre->Lock();
	FillPopup();
	((Resizer*)be_app)->Fenetre->Unlock();

	//le menu qui affiche le popup de type de fichier
	DropDownMenu = new BMenuField("DropTranslator", "Format:", Popup);

	//Bouton pour reinitialiser l'image
	Reset = new BButton("Reset", "Reset", new BMessage(RESET));

	//Bouton pour undo
	Undo = new BButton("Undo", "Undo", new BMessage(UNDO));

	//Bouton pour smooth scaller l'image 
	Smooth = new BButton("Smooth", "Smooth scaling", new BMessage(SMOOTH));

	//Tout les effets possibles...
	PopupEffect = new BPopUpMenu("Action");
	FillPopupEffect();

	//le menu qui affiche le popup effect
	DropDownEffect = new BMenuField("Effect", "", PopupEffect);
	DropDownEffect->SetDivider(0);

	//Bouton Grip
	Grip = new BButton("Grip", "Grip", new BMessage(GRIP));

	//Bouton Apply
	Apply = new BButton("Apply", "Apply", new BMessage(APPLY));

	//Bouton Coord
	Coord = new BCheckBox("Coord", "Coordinates window", new BMessage(COORD));

	//Bouton About
	Web = new BButton("About", "About", new BMessage(B_ABOUT_REQUESTED));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(Largeur->CreateLabelLayoutItem(), 0, 0)
			.Add(Largeur->CreateTextViewLayoutItem(), 1, 0)
//			.Add(SaveW, 2, 0)
//			.Add(LoadW, 3, 0)
			.Add(Hauteur->CreateLabelLayoutItem(), 0,1)
			.Add(Hauteur->CreateTextViewLayoutItem(), 1, 1)
//			.Add(SaveH, 2, 1)
//			.Add(LoadH, 3, 1)
			.Add(CheckBox, 1, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING), 0, 3)
			.Add(FileName->CreateLabelLayoutItem(), 0, 4)
			.Add(FileName->CreateTextViewLayoutItem(), 1, 4)
			.Add(DropDownMenu->CreateLabelLayoutItem(), 0, 5)
			.Add(DropDownMenu->CreateMenuBarLayoutItem(), 1, 5)
		.End()
		.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING))
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(Coord, 0, 0, 2)
			.Add(Reset, 0, 1)
			.Add(Undo, 1, 1)
			.Add(Smooth, 0, 2, 2)
			.Add(DropDownEffect, 0, 3, 2)
			.Add(Web, 0, 4)
			.Add(Apply, 1, 4)
		.End();

	Smooth->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	Reset->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	Undo->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	Web->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	Apply->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	Coord->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
}
//----------------------------------------------------------------------
void OptionView::FillPopup()
{
	//On load les tranlators
	int32 count = 0;
	status_t err = BTranslatorRoster::Default()->GetAllTranslators(&((Resizer*)be_app)->Fenetre->Main->all_translators, &count);

	if(err >= B_OK)
	{//pas d'erreur en allant chercher tous les translators
		err = B_ERROR; //on suppose la loi de murphy
		for (int i=0; i<count; i++)
		{//pour chaque translator...
			const translation_format* FormatIn;
			int32 FormatInCount;
			if(BTranslatorRoster::Default()->GetInputFormats(((Resizer*)be_app)->Fenetre->Main->all_translators[i], &FormatIn, &FormatInCount) >= B_OK) 
			{//pas de probleme a recuperer tous les inputs formats
				for(int j=0; j<FormatInCount; j++) //pour chaque format en input
				{
					if(FormatIn[j].type == B_TRANSLATOR_BITMAP)
					{//donc il peut lire des bitmaps en entree, donc on le veut...
						const translation_format* FormatOut;
						int32 FormatOutCount;
						if(BTranslatorRoster::Default()->GetOutputFormats(((Resizer*)be_app)->Fenetre->Main->all_translators[i], &FormatOut, &FormatOutCount) >= B_OK)
						{//on prends tous ceux qui ne ne donnent pas des Bitmap en sortie
							for(int k=0; k<FormatOutCount; k++) //On verifie ce qu'il produit en sortie
							{
								if(FormatOut[k].type != B_TRANSLATOR_BITMAP)
								{//on en fait un item dans notre menu
									BMessage* Message = new BMessage(CHANGE_OUTPUT);
									Message->AddInt16("translator", i);
									Message->AddInt16("output", k);
									BMenuItem* menuItem;
									Popup->AddItem(menuItem = new BMenuItem(FormatOut[k].name, Message));
									err = B_OK; //on a au moins un bon translator
									// sensible default
									if (strcmp(FormatOut[k].MIME, "image/png") == 0) {
										menuItem->SetMarked(true);
										((Resizer*)be_app)->Fenetre->Main->CurrentTranslator = i;
										((Resizer*)be_app)->Fenetre->Main->CurrentOutput = k;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(err != B_OK) //< originellement
		Window()->PostMessage(B_QUIT_REQUESTED); //ca devrait pas se produire
}
//----------------------------------------------------------------------
void OptionView::SetHauteur(int H)
{
	BString temp;
	temp << ((uint32)H);
	Hauteur->SetText(temp.String());
}
//----------------------------------------------------------------------
void OptionView::SetLargeur(int W)
{
	BString temp;
	temp << ((uint32)W);
	Largeur->SetText(temp.String());
}
//----------------------------------------------------------------------
void OptionView::ChangeEffect(int effect)
{
	CurrentEffect = effect;
}
//----------------------------------------------------------------------
void OptionView::FillPopupEffect()
{
	BMessage* Rotation = new BMessage(CHANGE_EFFECT);
	Rotation->AddInt16("Effect", ROTATE);
	PopupEffect->AddItem(new BMenuItem("90 Rotation", Rotation));

	BMessage* FlipH = new BMessage(CHANGE_EFFECT);
	FlipH->AddInt16("Effect", FLIPH);
	PopupEffect->AddItem(new BMenuItem("Flip Top-Bottom", FlipH));

	BMessage* FlipV = new BMessage(CHANGE_EFFECT);
	FlipV->AddInt16("Effect", FLIPV);
	PopupEffect->AddItem(new BMenuItem("Flip Left-Right", FlipV));

	BMessage* Light = new BMessage(CHANGE_EFFECT);
	Light->AddInt16("Effect", LIGHT);
	PopupEffect->AddItem(new BMenuItem("Light", Light));

	BMessage* Dark = new BMessage(CHANGE_EFFECT);
	Dark->AddInt16("Effect", DARK);
	PopupEffect->AddItem(new BMenuItem("Darkness", Dark));

	BMessage* Blur = new BMessage(CHANGE_EFFECT);
	Blur->AddInt16("Effect", BLUR);
	PopupEffect->AddItem(new BMenuItem("Blur", Blur));

	BMessage* Melt = new BMessage(CHANGE_EFFECT);
	Melt->AddInt16("Effect", MELT);
	PopupEffect->AddItem(new BMenuItem("Melt", Melt));

	BMessage* Drunk = new BMessage(CHANGE_EFFECT);
	Drunk->AddInt16("Effect", DRUNK);
	PopupEffect->AddItem(new BMenuItem("Drunk", Drunk));

	BMessage* Baw = new BMessage(CHANGE_EFFECT);
	Baw->AddInt16("Effect", BAW);
	PopupEffect->AddItem(new BMenuItem("Black & White", Baw));

	BMessage* Invert = new BMessage(CHANGE_EFFECT);
	Invert->AddInt16("Effect", INVERT);
	PopupEffect->AddItem(new BMenuItem("Invert", Invert));

	BMessage* Srg = new BMessage(CHANGE_EFFECT);
	Srg->AddInt16("Effect", SRG);
	PopupEffect->AddItem(new BMenuItem("Swap Red-Green", Srg));

	BMessage* Srb = new BMessage(CHANGE_EFFECT);
	Srb->AddInt16("Effect", SRB);
	PopupEffect->AddItem(new BMenuItem("Swap Red-Blue", Srb));

	BMessage* Sgb = new BMessage(CHANGE_EFFECT);
	Sgb->AddInt16("Effect", SGB);
	PopupEffect->AddItem(new BMenuItem("Swap Green-Blue", Sgb));

	BMessage* Screenshot = new BMessage(CHANGE_EFFECT);
	Screenshot->AddInt16("Effect", SCREENSHOT);
	PopupEffect->AddItem(new BMenuItem("Screenshot", Screenshot));
}
//----------------------------------------------------------------------
void OptionView::ApplyEffect()
{
	if(CurrentEffect != -1)
		Window()->PostMessage(new BMessage(CurrentEffect));
}
//----------------------------------------------------------------------
