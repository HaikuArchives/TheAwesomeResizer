#ifndef MOUSEWINDOW_H
#define MOUSEWINDOW_H

#include <Window.h>
#include "MouseView.h"
#include "enum.h"

class MouseWindow : public BWindow
{
 public:
	bool IsVisible;

	MouseView* fMouseView;
	MouseWindow();
	virtual void MessageReceived(BMessage* message);
};

#endif
