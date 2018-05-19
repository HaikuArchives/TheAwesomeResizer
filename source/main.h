#ifndef MAIN_H
#define MAIN_H

#include <Application.h> //BApplication
#include <Entry.h>
#include "MainWindow.h" //la fenetre de l'application
#include "OptionWindow.h"
#include "MouseWindow.h"

class Resizer : public BApplication
{
 public:
 	MainWindow* Fenetre;
	OptionWindow* Option;
	MouseWindow* Mouse;
	Resizer(); //constructeur de l'application
	virtual void RefsReceived( BMessage *message );
};

#endif
