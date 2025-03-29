#ifndef OPTIONWINDOW_H
#define OPTIONWINDOW_H

#include "Constants.h"
#include "OptionView.h"
#include <Window.h>

class OptionWindow : public BWindow {
public:
	int fLockW;
	int fLockH;

	OptionView* fOptionView;

	OptionWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage* message);
};

#endif
