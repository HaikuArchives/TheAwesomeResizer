#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <TranslationUtils.h>
#include <Bitmap.h>
#include <String.h>
#include <View.h>
#include "enum.h"
#include <deque>

using std::deque;

typedef deque<BBitmap*>::iterator ITER;

class MainView : public BView
{
	BBitmap* FirstBitmap; //le bitmap qui a ete drope au debut
	BBitmap* OriginalBitmap; //bitmap courrant
	BBitmap* ModifiedBitmap; //utilise pour le clipping
	BBitmap* offscreenBitmap; //utilise pour le offscreen drawing
	BView* offscreenView;
	deque<BBitmap*> file; //pour les undo
	bool DontResize;
	double Ratio;
	bool KeepRatio;
	char* FileName;
	BPoint Clipping1;
	BPoint Clipping2;
	bool dragging;

 public:
	translator_id* all_translators;
	int CurrentTranslator;
	int CurrentOutput;
	
	MainView();
	~MainView();
	bool GetImage(const char* path);
	virtual void Draw(BRect);
	virtual void MessageReceived(BMessage *message);
	virtual void MouseDown(BPoint where);
	virtual void MouseUp(BPoint where);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage* message);	
	void SmoothScale(BBitmap* origin, BBitmap* destination);
	void ToggleRatio();
	void ResetImage();
	void Copy(BMessage*);
	void ResizeImage();
	void ResizeImage(int width, int height);
	void RotateImage();
	void Flip(bool horizontal);
	BRect GetRegion();
	void Melt();
	void Dark();
	void Light();
	void Blur();
	void Drunk();
	void Invert();
	void GrabScreen();
	void BlackAndWhite();
	void InverseRG();
	void InverseRB();
	void InverseGB();
	void SmoothScale();
	void AddBitmap(BBitmap* B); //add a bitmap to the deque
	void Undo(); //Go back to previous image in deque
	void Flush(); //Flush all image in the deque
};

#endif
