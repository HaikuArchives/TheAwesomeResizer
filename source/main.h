/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *	2018-2025 The HaikuArchives Team
 *
 */
#ifndef MAIN_H
#define MAIN_H

#include "MainWindow.h"
#include "MouseWindow.h"
#include "OptionWindow.h"
#include <Application.h>
#include <Entry.h>

class Resizer : public BApplication {
public:
	virtual void RefsReceived(BMessage* message);

	MainWindow* fMainWin;
	OptionWindow* fOptionWin;
	MouseWindow* fMouseWin;
	Resizer();
};

#endif
