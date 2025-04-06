/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#ifndef OPTIONWINDOW_H
#define OPTIONWINDOW_H

#include "Constants.h"

#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <TranslatorRoster.h>
#include <Window.h>


class OptionWindow : public BWindow {
public:
	int fLockW;
	int fLockH;

	OptionWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage* message);

	BCheckBox* fSmoothBox;
	BTextControl* fFileName;
	BTextControl* fHeightTextbox;
	BTextControl* fWidthTextbox;

private:
	void _ApplyEffect();
	void _ChangeEffect(int effect);
	void _CreateEffectPopup();
	void _CreateFormatPopup();
	void _SetHeight(int height);
	void _SetWidth(int width);
	void _ShowTranslatorSettings();

	int fSavedHeight;
	int fSavedWidth;
	int fCurrentEffect;

	BCheckBox* fAspectBox;
	// BCheckBox* Coord;
	// BCheckBox* Grip;
	BButton* fResetButton;
	BButton* fUndoButton;
	BButton* fApplyButton;
	// BButton* fWebButton;
	BMenuField* fFormatMenu;
	BPopUpMenu* fFormatPopup;
	BMenuField* fEffectMenu;
	BPopUpMenu* fEffectPopup;
	BWindow* fTranslatorSettingsWindow;
};

#endif
