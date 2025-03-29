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
