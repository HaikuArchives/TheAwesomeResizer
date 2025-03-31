/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#include "OptionWindow.h"
#include "main.h"

#include <AboutWindow.h>
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <TranslatorFormats.h>

#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "OptionView"


OptionWindow::OptionWindow()
	:
	BWindow(BRect(10, 30, 10, 30), B_TRANSLATE_SYSTEM_NAME("The Awesome Resizer"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fLockH = 0;
	fLockW = 0;
	fSavedWidth = 640;
	fSavedHeight = 480;
	fCurrentEffect = -1;

	fWidthTextbox = new BTextControl(B_TRANSLATE("Width"), B_EMPTY_STRING, "", NULL);
	fWidthTextbox->SetModificationMessage(new BMessage(MOD_WIDTH));

	fHeightTextbox = new BTextControl(B_TRANSLATE("Height"), B_EMPTY_STRING, "", NULL);
	fHeightTextbox->SetModificationMessage(new BMessage(MOD_HEIGHT));

	fAspectBox = new BCheckBox("Ratio", B_TRANSLATE("Preserve aspect ratio"), new BMessage(RATIO));
	fAspectBox->SetValue(B_CONTROL_ON);

	fFileName = new BTextControl("Filename", B_TRANSLATE("Filename:"), "", NULL);

	fFormatPopup = new BPopUpMenu(B_TRANSLATE("Choose"));
	((Resizer*)be_app)->fMainWin->Lock();
	_CreateFormatPopup();
	((Resizer*)be_app)->fMainWin->Unlock();
	fFormatMenu = new BMenuField("DropTranslator", B_TRANSLATE("Format:"), fFormatPopup);

	fResetButton = new BButton("Reset", B_TRANSLATE("Reset"), new BMessage(RESET));
	fUndoButton = new BButton("Undo", B_TRANSLATE("Undo"), new BMessage(UNDO));

	fSmoothBox = new BCheckBox("Smooth", B_TRANSLATE("Smooth scaling"), new BMessage(SMOOTH));
	// Smooth->SetValue(B_CONTROL_ON);
	fEffectPopup = new BPopUpMenu(B_TRANSLATE("Choose an action"));
	_CreateEffectPopup();
	fEffectMenu = new BMenuField("Effect", /*"Action:"*/ B_EMPTY_STRING, fEffectPopup);

	// Bouton Grip
	//  Grip = new BCheckBox("Grip", "Show grip", new BMessage(GRIP));

	fApplyButton = new BButton("Apply", B_TRANSLATE("Apply"), new BMessage(APPLY));

	// Bouton Coord
	//  Coord = new BCheckBox("Coord", B_TRANSLATE("Coordinates window"), new BMessage(COORD));

	// Bouton About
	//  fWebButton = new BButton("About", B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
			B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_ITEM_SPACING)
		.Add(new BStringView("Size", B_TRANSLATE("Size:")))
		.Add(fWidthTextbox)
		.Add(new BStringView("separator", "x"))
		.Add(fHeightTextbox)
		.End()
		.Add(fAspectBox)
		.Add(fSmoothBox)
		//			.Add(Grip)
		//			.Add(Coord)
		.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING))
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
		.Add(fFileName->CreateLabelLayoutItem(), 0, 0)
		.Add(fFileName->CreateTextViewLayoutItem(), 1, 0)
		.Add(fFormatMenu->CreateLabelLayoutItem(), 0, 1)
		.Add(fFormatMenu->CreateMenuBarLayoutItem(), 1, 1)
		.End()
		.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING))

		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
		.Add(fResetButton, 0, 0)
		.Add(fUndoButton, 1, 0)
		//			.Add(Web, 0, 1)
		.Add(fEffectMenu, 0, 1, 2)
		.Add(fApplyButton, 0, 2, 2)
		.End();

	fSmoothBox->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fResetButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fUndoButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	// fWebButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fApplyButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	// Coord->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
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
			fSavedHeight = atoi(fHeightTextbox->Text());
			break;
		case SAVEW:
			fSavedWidth = atoi(fWidthTextbox->Text());
			break;
		case LOADH:
			_SetHeight(fSavedHeight);
			break;
		case LOADW:
			_SetWidth(fSavedWidth);
			break;
		case CHANGE_EFFECT:
			_ChangeEffect(message->FindInt16("Effect"));
			break;
		case APPLY:
			_ApplyEffect();
			break;

		case RESET:
		{
			fAspectBox->SetValue(B_CONTROL_ON);
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
			const char* temp = fWidthTextbox->Text();
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
			const char* temp = fHeightTextbox->Text();
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
			_SetWidth(message->FindInt16("NewWidth"));
			PostMessage(UNLOCK_W, NULL);
		} break;

		case TEXT_HEIGHT:
		{ // same as above
			fLockH++;
			_SetHeight(message->FindInt16("NewHeight"));
			PostMessage(UNLOCK_H, NULL);
		} break;

		case UNDO_OK:
		{
			fUndoButton->SetEnabled(true);
		} break;

		case UNDO_NOT_OK:
		{
			fUndoButton->SetEnabled(false);
		} break;

		default:
			BWindow::MessageReceived(message);
	}
}


void
OptionWindow::_ApplyEffect()
{
	if (fCurrentEffect != -1)
		PostMessage(new BMessage(fCurrentEffect));
}


void
OptionWindow::_ChangeEffect(int effect)
{
	fCurrentEffect = effect;
}


void
OptionWindow::_CreateEffectPopup()
{
	BMessage* Rotation = new BMessage(CHANGE_EFFECT);
	Rotation->AddInt16("Effect", ROTATE);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("90° rotation"), Rotation));

	BMessage* FlipH = new BMessage(CHANGE_EFFECT);
	FlipH->AddInt16("Effect", FLIPH);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Flip top-bottom"), FlipH));

	BMessage* FlipV = new BMessage(CHANGE_EFFECT);
	FlipV->AddInt16("Effect", FLIPV);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Flip left-right"), FlipV));

	BMessage* Light = new BMessage(CHANGE_EFFECT);
	Light->AddInt16("Effect", LIGHT);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Light"), Light));

	BMessage* Dark = new BMessage(CHANGE_EFFECT);
	Dark->AddInt16("Effect", DARK);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Darkness"), Dark));

	BMessage* Blur = new BMessage(CHANGE_EFFECT);
	Blur->AddInt16("Effect", BLUR);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Blur"), Blur));

	BMessage* Melt = new BMessage(CHANGE_EFFECT);
	Melt->AddInt16("Effect", MELT);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Melt"), Melt));

	BMessage* Drunk = new BMessage(CHANGE_EFFECT);
	Drunk->AddInt16("Effect", DRUNK);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Drunk"), Drunk));

	BMessage* Baw = new BMessage(CHANGE_EFFECT);
	Baw->AddInt16("Effect", BAW);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Grayscale"), Baw));

	BMessage* Invert = new BMessage(CHANGE_EFFECT);
	Invert->AddInt16("Effect", INVERT);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Invert"), Invert));

	BMessage* Srg = new BMessage(CHANGE_EFFECT);
	Srg->AddInt16("Effect", SRG);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Swap red-green"), Srg));

	BMessage* Srb = new BMessage(CHANGE_EFFECT);
	Srb->AddInt16("Effect", SRB);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Swap red-blue"), Srb));

	BMessage* Sgb = new BMessage(CHANGE_EFFECT);
	Sgb->AddInt16("Effect", SGB);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Swap green-blue"), Sgb));

	BMessage* Screenshot = new BMessage(CHANGE_EFFECT);
	Screenshot->AddInt16("Effect", SCREENSHOT);
	fEffectPopup->AddItem(new BMenuItem(B_TRANSLATE("Screenshot"), Screenshot));
}


void
OptionWindow::_CreateFormatPopup()
{
	int32 count = 0;
	status_t err = BTranslatorRoster::Default()->GetAllTranslators(
		&((Resizer*)be_app)->fMainWin->fMainView->all_translators, &count);

	if (err >= B_OK) {
		err = B_ERROR;
		for (int i = 0; i < count; i++) {
			const translation_format* FormatIn;
			int32 FormatInCount;
			if (BTranslatorRoster::Default()->GetInputFormats(
					((Resizer*)be_app)->fMainWin->fMainView->all_translators[i], &FormatIn,
					&FormatInCount)
				>= B_OK) {
				for (int j = 0; j < FormatInCount; j++) {
					if (FormatIn[j].type == B_TRANSLATOR_BITMAP) {
						const translation_format* FormatOut;
						int32 FormatOutCount;
						if (BTranslatorRoster::Default()->GetOutputFormats(
								((Resizer*)be_app)->fMainWin->fMainView->all_translators[i],
								&FormatOut, &FormatOutCount)
							>= B_OK) {
							for (int k = 0; k < FormatOutCount; k++) {
								if (FormatOut[k].type != B_TRANSLATOR_BITMAP) {
									BMessage* Message = new BMessage(CHANGE_OUTPUT_FORMAT);
									Message->AddInt16("translator", i);
									Message->AddInt16("output", k);
									BMenuItem* menuItem;
									fFormatPopup->AddItem(
										menuItem = new BMenuItem(FormatOut[k].name, Message));
									err = B_OK;
									// set default format
									if (strcmp(FormatOut[k].MIME, "image/png") == 0) {
										menuItem->SetMarked(true);
										((Resizer*)be_app)->fMainWin->fMainView->fCurrentTranslator
											= i;
										((Resizer*)be_app)->fMainWin->fMainView->fCurrentOutput = k;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (err != B_OK)
		PostMessage(B_QUIT_REQUESTED); // should never happen
}


void
OptionWindow::_SetHeight(int height)
{
	BString temp;
	temp << ((uint32)height);
	fHeightTextbox->SetText(temp.String());
}


void
OptionWindow::_SetWidth(int width)
{
	BString temp;
	temp << ((uint32)width);
	fWidthTextbox->SetText(temp.String());
}


