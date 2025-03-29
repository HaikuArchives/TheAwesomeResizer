#ifndef OPTIONWINDOW_H
#define OPTIONWINDOW_H

#include <Window.h>
#include "OptionView.h"
#include "enum.h"

class OptionWindow : public BWindow
{
 public:
	int fLockW;
	int fLockH;

	OptionView* fOptionView;

	OptionWindow();
	virtual	bool QuitRequested();
	virtual void MessageReceived(BMessage* message);
};

#endif
