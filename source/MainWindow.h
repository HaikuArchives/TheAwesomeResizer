#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MainView.h"
#include "Constants.h"

#include <Window.h>

class MainWindow : public BWindow {
public:
	bool fBigGrip;
	bool fDontUpdate;
	MainView* fMainView;

	MainWindow();
	virtual bool QuitRequested();
	virtual void FrameResized(float W, float H);
	virtual void MessageReceived(BMessage* message);
};

#endif
