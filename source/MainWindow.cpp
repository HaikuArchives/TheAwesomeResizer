#include "MainWindow.h"
#include <Application.h>
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"

//-------------------------------------------------------------------
MainWindow::MainWindow()
	: BWindow(BRect(240, 30, 440, 130), B_TRANSLATE("TA: (No image)"),
	B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_WILL_ACCEPT_FIRST_CLICK)
{
	Main = new MainView();
	AddChild(Main);
	DontUpdate = false;
	BigGrip = false;
}
//-------------------------------------------------------------------
bool MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true; //on permet de quitter
}
//-------------------------------------------------------------------
void MainWindow::FrameResized(float W, float H)
{
	Main->ResizeImage(); //on demande de resizer
}
//-------------------------------------------------------------------
void MainWindow::MessageReceived(BMessage * message)
{
	switch(message->what)
	{
		//fait la copie vers le tracker
		case B_COPY_TARGET: Main->Copy(message);break;

		//toggle le ratio
		case RATIO: Main->ToggleRatio();break;

		//reset l'image
		case RESET: Main->ResetImage();break;

		//undo derniere action
		case UNDO: Main->Undo();break;

		//Smooth scale l'image
		case SMOOTH: Main->Invalidate();break;

		//rotation 90 degree
		case ROTATE: Main->RotateImage();break;

		//flip horizontal
		case FLIPH: Main->Flip(true);break;

		//flip vertical
		case FLIPV: Main->Flip(false);break;

		//Assombrit
		case DARK: Main->Dark();break;

		//Eclaircit
		case LIGHT: Main->Light();break;

		//Super Blur
		case BLUR: Main->Blur();break;

		//Black & White
		case BAW: Main->BlackAndWhite();break;

		//Pogne un screen shot de l'ecran sans le resizer
		case SCREENSHOT: Main->GrabScreen();break;

		//Effet Melt sur une image
		case MELT: Main->Melt();break;

		//Invert color
		case INVERT: Main->Invert();break;

		//Inverse Rouge et vert
		case SRG: Main->InverseRG();break;

		//Inverse Rouge et bleu
		case SRB: Main->InverseRB();break;

		//Inverse Vert et bleu
		case SGB: Main->InverseGB();break;

		//Pogne un screen shot de l'ecran sans le resizer
		case DRUNK: Main->Drunk();break;

		//Toggle la grip de la fenetre
		case GRIP:
		{
			if(BigGrip)	SetLook(B_TITLED_WINDOW_LOOK);
			else		SetLook(B_DOCUMENT_WINDOW_LOOK);
			BigGrip = !BigGrip;
		}break;

		//Change le type d'image en output
		case CHANGE_OUTPUT:
		{
			Main->CurrentTranslator = message->FindInt16("translator");
			Main->CurrentOutput = message->FindInt16("output");
		}break;

		//Change Largeur
		case MOD_WIDTH :
		{
			DontUpdate = true; //pas renvoyer une valeur
			Main->ResizeImage(message->FindInt16("NewWidth"), -1);
		}break;

		//Change Hauteur
		case MOD_HEIGHT :
		{
			DontUpdate = true; //pas renvoyer une valeurs
			Main->ResizeImage(-1, message->FindInt16("NewHeight"));
		}break;

		default:BWindow::MessageReceived(message);break;
	}
	//SetTitle(B_TRANSLATE("TAR: (No image)"));
	UpdateIfNeeded();
}
//-------------------------------------------------------------------
