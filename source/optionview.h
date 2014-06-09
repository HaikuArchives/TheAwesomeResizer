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

class OptionView : public BBox
{
 public:
	BTextControl* Hauteur;
	BTextControl* Largeur;
	BTextControl* FileName;
	BButton* SaveW;
	BButton* SaveH;
	BButton* LoadH;
	BButton* LoadW;
	BCheckBox* CheckBox;
	BButton* Reset;
	BButton* Undo;
	BButton* Smooth;
	BMenuField* DropDownMenu;
	BPopUpMenu* Popup;	
	BMenuField* DropDownEffect;
	BPopUpMenu* PopupEffect;
	BButton* Apply;
	BButton* Web;
	BButton* Coord;
	BButton* Grip;
	int SavedH;
	int SavedW;
	int CurrentEffect;

	OptionView();
	void SetHauteur(int H);
	void SetLargeur(int W);
	void ChangeEffect(int effect);
	void ApplyEffect();
	void FillPopup();
	void FillPopupEffect();
};

#endif