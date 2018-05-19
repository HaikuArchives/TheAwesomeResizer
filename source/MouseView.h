#ifndef MOUSEVIEW_H
#define MOUSEVIEW_H

#include <StringView.h>
#include "enum.h"

class MouseView : public BView
{
 private:
	BStringView* Coord;
	BStringView* Size;
	int	Clip1x, Clip1y, Clip2x, Clip2y;
	
 public:
	MouseView();
	void ShowCoord(float x, float y);
	void ClearClip();
	void Clip1(float x, float y);
	void Clip2(float x, float y);
};

#endif
