/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *
 */
#ifndef MOUSEWINDOW_H
#define MOUSEWINDOW_H

#include "MouseView.h"

#include <Window.h>

class MouseWindow : public BWindow {
public:
	bool IsVisible;

	MouseView* fMouseView;
	MouseWindow();
	virtual void MessageReceived(BMessage* message);
};

#endif
