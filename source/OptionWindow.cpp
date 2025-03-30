/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#include "OptionView.h"
#include "main.h"

#include <AboutWindow.h>
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>

#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OptionWindow"


OptionWindow::OptionWindow()
	:
	BWindow(BRect(10, 30, 10, 30), B_TRANSLATE_SYSTEM_NAME("The Awesome Resizer"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fOptionView = new OptionView();
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0).Add(fOptionView);
	fLockH = 0;
	fLockW = 0;
}


bool
OptionWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
OptionWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case RATIO: // redirect message to MainWindow
		case CHANGE_OUTPUT_FORMAT:
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
			((Resizer*)be_app)->fMainWin->PostMessage(message);
			break;

		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* aboutWin
				= new BAboutWindow(B_TRANSLATE("TAResizer"), "application/x-vnd.TAResizer");
			aboutWin->AddDescription(
				B_TRANSLATE("A simple application that allows quick dynamic"
							" resizing of any translator supported image and much much more."));
			aboutWin->AddCopyright(1999, "Jonathan Villemure");
			const char* authors[] = {"Jonathan Villemure", NULL};
			aboutWin->AddAuthors(authors);
			aboutWin->Show();
		}
		case COORD:
			((Resizer*)be_app)->fMouseWin->PostMessage(message);
			break;
		case SAVEH:
			fOptionView->fSavedHeight = atoi(fOptionView->fHeightTextbox->Text());
			break;
		case SAVEW:
			fOptionView->fSavedWidth = atoi(fOptionView->fWidthTextbox->Text());
			break;
		case LOADH:
			fOptionView->SetHeight(fOptionView->fSavedHeight);
			break;
		case LOADW:
			fOptionView->SetWidth(fOptionView->fSavedWidth);
			break;
		case CHANGE_EFFECT:
			fOptionView->ChangeEffect(message->FindInt16("Effect"));
			break;
		case APPLY:
			fOptionView->ApplyEffect();
			break;

		case RESET:
		{
			fOptionView->fAspectBox->SetValue(B_CONTROL_ON);
			((Resizer*)be_app)->fMainWin->PostMessage(RESET);
		} break;

		case UNLOCK_H:
			fLockH--;
			break;
		case UNLOCK_W:
			fLockW--;
			break;

		case MOD_WIDTH:
		{
			if (fLockW > 0)
				return;
			int width;
			const char* temp = fOptionView->fWidthTextbox->Text();
			width = atoi(temp);
			if (width < 2049) {
				message->AddInt16("NewWidth", width);
				((Resizer*)be_app)->fMainWin->PostMessage(message);
			}
		} break;

		case MOD_HEIGHT:
		{
			if (fLockH > 0)
				return;
			int height;
			const char* temp = fOptionView->fHeightTextbox->Text();
			height = atoi(temp);
			if (height < 2049) {
				message->AddInt16("NewHeight", height);
				((Resizer*)be_app)->fMainWin->PostMessage(message);
			}
		} break;

		case TEXT_WIDTH:
		{ /* Sent by the MainWindow, it will modify the textbox, but
		  since it wasn't modified by the user, we must prevent
		  that modification message to be sent by locking the window */
			fLockW++;
			fOptionView->SetWidth(message->FindInt16("NewWidth"));
			PostMessage(UNLOCK_W, NULL);
		} break;

		case TEXT_HEIGHT:
		{ // same as above
			fLockH++;
			fOptionView->SetHeight(message->FindInt16("NewHeight"));
			PostMessage(UNLOCK_H, NULL);
		} break;

		case UNDO_OK:
		{
			fOptionView->fUndoButton->SetEnabled(true);
		} break;

		case UNDO_NOT_OK:
		{
			fOptionView->fUndoButton->SetEnabled(false);
		} break;

		default:
			BWindow::MessageReceived(message);
	}
}
