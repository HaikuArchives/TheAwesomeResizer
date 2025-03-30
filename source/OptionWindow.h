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
