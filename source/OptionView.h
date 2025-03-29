#ifndef OPTIONVIEW_H
#define OPTIONVIEW_H

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <TranslatorRoster.h>

class OptionView : public BView {
public:
	BTextControl* fHeightTextbox;
	BTextControl* fWidthTextbox;
	BTextControl* fFileName;
	BCheckBox* fAspectBox;
	BCheckBox* fSmoothBox;
	BButton* fResetButton;
	BButton* fUndoButton;
	BButton* fApplyButton;
	BButton* fWebButton;
	BMenuField* fFormatMenu;
	BPopUpMenu* fFormatPopup;
	BMenuField* fEffectMenu;
	BPopUpMenu* fEffectPopup;

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
