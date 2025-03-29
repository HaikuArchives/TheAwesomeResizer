#ifndef MAIN_H
#define MAIN_H

#include <Application.h>
#include <Entry.h>
#include "MainWindow.h"
#include "OptionWindow.h"
#include "MouseWindow.h"

class Resizer : public BApplication
{
 public:
 	MainWindow* fMainWin;
	OptionWindow* fOptionWin;
	MouseWindow* fMouseWin;
	Resizer();
	virtual void RefsReceived( BMessage *message );
};

#endif
