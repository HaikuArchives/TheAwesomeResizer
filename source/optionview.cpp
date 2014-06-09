#include "optionview.h"
#include <String.h>
#include <Application.h>
#include "main.h"
//----------------------------------------------------------------------
OptionView::OptionView() 
	: BBox(BRect(0, 0, 122, 257), "Box")
{
	SavedW = 640; SavedH = 480; CurrentEffect = -1;
	
	Largeur = new BTextControl(BRect(5, 5, 80, 5), "Width","Width", "", NULL);
	Largeur->SetModificationMessage(new BMessage(MOD_WIDTH)); 
	Largeur->SetDivider(35);
	AddChild(Largeur); //Boite qui indique la largeur

	Hauteur = new BTextControl(BRect(5, 30, 80, 30), "Height", "Height", "", NULL);
	Hauteur->SetModificationMessage(new BMessage(MOD_HEIGHT)); 
	Hauteur->SetDivider(35); 
	AddChild(Hauteur); //Boite qui indique la Hauteur

	SaveW = new BButton(BRect(80, 2, 100, 2), "SaveW", "S", new BMessage(SAVEW));
	AddChild(SaveW); //Save Largeur

	LoadW = new BButton(BRect(100, 2, 120, 2), "LoadW", "L", new BMessage(LOADW));
	AddChild(LoadW); //Load Largeur

	SaveH = new BButton(BRect(80, 27, 100, 27), "SaveH", "S", new BMessage(SAVEH));
	AddChild(SaveH); //Save Hauteur

	LoadH = new BButton(BRect(100, 27, 120, 27), "LoadH", "L", new BMessage(LOADH));
	AddChild(LoadH); //Load Hauteur

	CheckBox = new BCheckBox(BRect(5, 50, 110, 50), "Ratio", "Aspect Ratio", new BMessage(RATIO));
	CheckBox->SetValue(B_CONTROL_ON);
	AddChild(CheckBox); //CheckBox pour dire si on veut conserver le ratio

	FileName = new BTextControl(BRect(5, 80, 110, 80), "Filename","File", "", NULL);
	FileName->SetDivider(20);
	AddChild(FileName); //Boite qui permet de dire le nom du fichier output

	//Popup qui affiche tous les translators disponibles
	Popup = new BPopUpMenu("Choose");

 	//on barre l'autre fenetre le temps de faire ca
 	((Resizer*)be_app)->Fenetre->Lock();
	FillPopup();
	((Resizer*)be_app)->Fenetre->Unlock();

	//le menu qui affiche le popup de type de fichier
	DropDownMenu = new BMenuField(BRect(5, 100, 120, 100), "DropTranslator", "As", Popup);
	DropDownMenu->SetDivider(20);
	AddChild(DropDownMenu);

	//Bouton pour reinitialiser l'image
	Reset = new BButton(BRect(5, 125, 60, 125), "Reset", "Reset", new BMessage(RESET));
	AddChild(Reset);

	//Bouton pour undo
	Undo = new BButton(BRect(60, 125, 115, 125), "Undo", "Undo", new BMessage(UNDO));
	AddChild(Undo);

	//Bouton pour smooth scaller l'image 
	Smooth = new BButton(BRect(5, 150, 115, 150), "Smooth", "Smooth scaling", new BMessage(SMOOTH));
	AddChild(Smooth);

	//Tout les effets possibles...
	PopupEffect = new BPopUpMenu("Action");
	FillPopupEffect();

	//le menu qui affiche le popup effect
	DropDownEffect = new BMenuField(BRect(5, 175, 115, 175), "Effect", "", PopupEffect);
	DropDownEffect->SetDivider(0);
	AddChild(DropDownEffect);

	//Bouton Grip
	Grip = new BButton(BRect(5, 200, 60, 200), "Grip", "Grip", new BMessage(GRIP));
	AddChild(Grip);

	//Bouton Apply
	Apply = new BButton(BRect(60, 200, 115, 200), "Apply", "Do it !", new BMessage(APPLY));
	AddChild(Apply);

	//Bouton Coord
	Coord = new BButton(BRect(5, 225, 60, 225), "Coord", "Coord", new BMessage(COORD));
	AddChild(Coord);

	//Bouton About
	Web = new BButton(BRect(60, 225, 115, 225), "About", "About", new BMessage(ABOUT));
	AddChild(Web);
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
									Popup->AddItem(new BMenuItem(FormatOut[k].name, Message));
									err = B_OK; //on a au moins un bon translator
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
