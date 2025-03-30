/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#include "MainWindow.h"
#include "main.h"

#include <Application.h>
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


MainWindow::MainWindow()
	:
	BWindow(BRect(240, 30, 440, 130), B_TRANSLATE("TAR: (No image)"), B_DOCUMENT_WINDOW,
		B_NOT_ZOOMABLE | B_WILL_ACCEPT_FIRST_CLICK)
{
	fMainView = new MainView();
	AddChild(fMainView);
	fDontUpdate = false;
	fBigGrip = false;
}


bool
MainWindow::QuitRequested()
{
	if (((Resizer*)be_app)->fOptionWin && !((Resizer*)be_app)->fOptionWin->IsHidden()) {
		// If option window is still open, don't quit yet
		if (fMainView->HasImage()) {
			fMainView->ClearImage();
			return false;
		}
	}

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::FrameResized(float W, float H)
{
	fMainView->ResizeImage();
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		// fait la copie vers le tracker
		case B_COPY_TARGET:
			fMainView->Copy(message);
			break;

		case RATIO:
			fMainView->ToggleRatio();
			break;

		case RESET:
			fMainView->ResetImage();
			break;

		case UNDO:
			fMainView->Undo();
			break;

		case SMOOTH:
			fMainView->Invalidate();
			break;

		case ROTATE:
			fMainView->RotateImage();
			break;

		case FLIPH:
			fMainView->Flip(true);
			break;

		case FLIPV:
			fMainView->Flip(false);
			break;

		case DARK:
			fMainView->Dark();
			break;

		case LIGHT:
			fMainView->Light();
			break;

		case BLUR:
			fMainView->Blur();
			break;

		case BAW:
			fMainView->BlackAndWhite();
			break;

		case SCREENSHOT:
			fMainView->GrabScreen();
			break;

		case MELT:
			fMainView->Melt();
			break;

		case INVERT:
			fMainView->Invert();
			break;

		case SRG:
			fMainView->InverseRG();
			break;

		case SRB:
			fMainView->InverseRB();
			break;

		case SGB:
			fMainView->InverseGB();
			break;

		case DRUNK:
			fMainView->Drunk();
			break;

		case GRIP:
		{
			if (fBigGrip)
				SetLook(B_TITLED_WINDOW_LOOK);
			else
				SetLook(B_DOCUMENT_WINDOW_LOOK);
			fBigGrip = !fBigGrip;
		} break;

		case CHANGE_OUTPUT_FORMAT:
		{
			fMainView->fCurrentTranslator = message->FindInt16("translator");
			fMainView->fCurrentOutput = message->FindInt16("output");
		} break;

		case MOD_WIDTH:
		{
			fDontUpdate = true; // don't return a value
			fMainView->ResizeImage(message->FindInt16("NewWidth"), -1);
		} break;

		case MOD_HEIGHT:
		{
			fDontUpdate = true; // don't return a value
			fMainView->ResizeImage(-1, message->FindInt16("NewHeight"));
		} break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
	UpdateIfNeeded();
}
