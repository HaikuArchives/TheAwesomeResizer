#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <Bitmap.h>
#include <FilePanel.h>
#include <String.h>
#include <TranslationUtils.h>
#include <View.h>

#include <deque>

using std::deque;

typedef deque<BBitmap*>::iterator ITER;

class MainView : public BView {
	BBitmap* fFirstBitmap; // the Bitmap dropped a launch
	BBitmap* fOriginalBitmap; // current bitmap
	BBitmap* fModifiedBitmap; // used for clipping
	BBitmap* fOffscreenBitmap; // used for offscreen drawing
	BView* fOffscreenView;
	deque<BBitmap*> fFile; // used for undo
	bool fDontResize;
	double fRatio;
	bool fKeepRatio;
	char* fFileName;
	BPoint fClipPoint1;
	BPoint fClipPoint2;
	bool fDragging;

public:
	translator_id* all_translators;
	int fCurrentTranslator;
	int fCurrentOutput;

	MainView();
	~MainView();

	virtual void Draw(BRect);
	virtual void MessageReceived(BMessage* message);
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

	void AddBitmap(BBitmap* B); // add a bitmap to the deque
	void Undo(); // Go back to previous image in deque
	void Flush(); // Flush all images in the deque

	bool HasImage() { return fOriginalBitmap != NULL; }
	void ClearImage();
	bool GetImage(const char* path);
	BRect GetRegion();
private:
	BFilePanel* fOpenPanel;
};

#endif
