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
