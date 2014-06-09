#ifndef MOUSEW_H
#define MOUSEW_H

#include <Window.h> //BWindow 
#include "mouseview.h"
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
