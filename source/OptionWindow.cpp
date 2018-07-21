#include "OptionView.h"
#include <AboutWindow.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include "main.h"
#include <stdlib.h>
//-------------------------------------------------------------------
OptionWindow::OptionWindow()
	: BWindow(BRect(10, 30, 10, 30), "Options", B_TITLED_WINDOW,
	  B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	Option = new OptionView();
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(Option);
	LockH = 0;
	LockW = 0;
}
//-------------------------------------------------------------------
bool OptionWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true; //on permet de quitter
}
//-------------------------------------------------------------------
void OptionWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case RATIO: //redirige le message vers l'autre fenetre
		case CHANGE_OUTPUT:
		case ROTATE:
		case FLIPH:
		case FLIPV:
		case DARK:
		case LIGHT:
		case BLUR:
		case BAW:
		case SCREENSHOT:
		case SMOOTH:
		case UNDO:
		case MELT:
		case DRUNK:
		case INVERT:
		case SRG:
		case SRB:
		case SGB:
		case GRIP:
		case DESKBKG:
			((Resizer*)be_app)->Fenetre->PostMessage(message);break;

		case B_ABOUT_REQUESTED: {
			BAboutWindow* aboutWin = new BAboutWindow("TAResizer", "application/x-vnd.TAResizer");
			aboutWin->AddDescription("A simple application that allows quick dynamic resizing of any translator"
			" supported image and much much more.");
			aboutWin->AddCopyright(1999, "Jonathan Villemure");
			const char* authors[] = {
				"Jonathan Villemure",
				NULL
			};
			aboutWin->AddAuthors(authors);
			aboutWin->Show();
		}
		case COORD: ((Resizer*)be_app)->Mouse->PostMessage(message);break;
		case SAVEH:	Option->SavedH = atoi(Option->Hauteur->Text());break;
		case SAVEW:	Option->SavedW = atoi(Option->Largeur->Text());break;
		case LOADH:	Option->SetHauteur(Option->SavedH);break;
		case LOADW:	Option->SetLargeur(Option->SavedW);break;
		case CHANGE_EFFECT:	Option->ChangeEffect(message->FindInt16("Effect"));break;
		case APPLY:	Option->ApplyEffect();break;
		
		case RESET:
		{	//redirige le message vers l'autre fenetre
			Option->CheckBox->SetValue(B_CONTROL_ON); //coche par defaut
			((Resizer*)be_app)->Fenetre->PostMessage(RESET);
		}break;

		case UNLOCK_H: LockH--;break;
		case UNLOCK_W: LockW--;break;
		
		case MOD_WIDTH :
		{
			if(LockW > 0) return;
			int width;
			const char* temp = Option->Largeur->Text();
			width = atoi(temp);
			if(width < 2049)
			{
				message->AddInt16("NewWidth", width);
				((Resizer*)be_app)->Fenetre->PostMessage(message);
			}
		}break;

		case MOD_HEIGHT :
		{
			if(LockH > 0) return;
			int height;
			const char* temp = Option->Hauteur->Text();
			height = atoi(temp);
			if(height < 2049)
			{
				message->AddInt16("NewHeight", height);
				((Resizer*)be_app)->Fenetre->PostMessage(message);
			}
		}break;

		case TEXT_WIDTH:
		{/*nous est envoye par la fenetre principale, ca va causer une 
		modification au text box mais on doit pas dire a la fenetre 
		principale de se resizer pcq c'est pas l'user qui a modifier 
		les valeurs*/
			LockW++;
			Option->SetLargeur(message->FindInt16("NewWidth"));		
			PostMessage(UNLOCK_W, NULL);
		}break;

		case TEXT_HEIGHT:
		{//voir notes de la case d'avant
			LockH++;
			Option->SetHauteur(message->FindInt16("NewHeight"));		
			PostMessage(UNLOCK_H, NULL);
		}break;
		
		case UNDOOK:
		{//enable undo button
			Option->Undo->SetEnabled(true);
		}break;
		
		case UNDONOK:
		{//disable undo button
			Option->Undo->SetEnabled(false);
		}break;
		
		default: BWindow::MessageReceived(message);
	}
}
//--------------------------------------------------------------------------------
