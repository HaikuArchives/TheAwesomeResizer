#ifndef MOUSEWINDOW_H
#define MOUSEWINDOW_H

#include <Window.h> //BWindow 
#include "MouseView.h"
#include "enum.h"

class MouseWindow : public BWindow 
{
 public:
	bool IsVisible;

	MouseView* MouseV;
	MouseWindow(); //constructeur
	virtual void MessageReceived(BMessage* message);
};

#endif
