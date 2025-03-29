#ifndef OPTIONVIEW_H
#define OPTIONVIEW_H

#include <Box.h>
#include <StringView.h>
#include <TextControl.h>
#include <CheckBox.h>
#include <Button.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <TranslatorRoster.h>
#include "enum.h"

class OptionView : public BView
{
 public:
	BTextControl* fHeightTextbox;
	BTextControl* fWidthTextbox;
	BTextControl* fFileName;
	BCheckBox* fAspectBox;
	BButton* fResetButton;
	BButton* fUndoButton;
	BCheckBox* fSmoothBox;
	BMenuField* fFormatMenu;
	BPopUpMenu* fFormatPopup;
	BMenuField* fEffectMenu;
	BPopUpMenu* fEffectPopup;
	BButton* fApplyButton;
	BButton* fWebButton;
	// BCheckBox* Coord;
	// BCheckBox* Grip;
	int fSavedHeight;
	int fSavedWidth;

	int fCurrentEffect;

	OptionView();
	void SetHeight(int height);
	void SetWidth(int width);
	void ChangeEffect(int effect);
	void ApplyEffect();

private:
	void _CreateFormatPopup();
	void _CreateEffectPopup();
};

#endif
