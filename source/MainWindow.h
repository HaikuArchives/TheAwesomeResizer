/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MainView.h"
#include "Constants.h"

#include <Window.h>

class MainWindow : public BWindow {
public:
	bool fBigGrip;
	bool fDontUpdate;
	MainView* fMainView;

	MainWindow();
	virtual bool QuitRequested();
	virtual void FrameResized(float W, float H);
	virtual void MessageReceived(BMessage* message);
};

#endif
