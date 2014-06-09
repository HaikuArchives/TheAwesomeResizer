#ifndef FENETRE_H
#define FENETRE_H

#include <Window.h> //BWindow 
#include "mainview.h"
#include "enum.h"

class MainWindow : public BWindow 
{
 public:
	bool BigGrip;
	bool DontUpdate;
	MainView* Main;

	MainWindow(); //constructeur
	virtual	bool QuitRequested(); //pour quitter
	virtual void FrameResized(float W, float H);
	virtual void MessageReceived(BMessage* message);
};

#endif
