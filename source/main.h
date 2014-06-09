#ifndef MAIN_H
#define MAIN_H

#include <Application.h> //BApplication
#include <Entry.h>
#include "fenetre.h" //la fenetre de l'application
#include "option.h"
#include "mousew.h"
#include "about.h"

class Resizer : public BApplication
{
 public:
 	MainWindow* Fenetre;
	OptionWindow* Option;
	MouseWindow* Mouse;
	AboutBox* About;
	Resizer(); //constructeur de l'application
	virtual void RefsReceived( BMessage *message );
};

#endif