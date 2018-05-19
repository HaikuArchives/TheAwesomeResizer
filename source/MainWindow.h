#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h> //BWindow 
#include "MainView.h"
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
