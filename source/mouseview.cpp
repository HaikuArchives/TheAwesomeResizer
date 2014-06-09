#include "mouseview.h"
#include <String.h>
#include <Application.h>
#include "main.h"
#include <math.h>
//----------------------------------------------------------------------
MouseView::MouseView() 
	: BBox(BRect(0, 0, 122, 232), "MouseBox")
{
	Coord = new BStringView(BRect(5, 5, 80, 20), "Coord", "X:0 Y:0");
	AddChild(Coord);

	Size = new BStringView(BRect(5, 20, 80, 35), "Size", "- x -");
	AddChild(Size);
}
//----------------------------------------------------------------------
void MouseView::ShowCoord(float x, float y)
{
	BString temp;
	if(x > 4096 || x < 0 || y > 4096 || y < 0)
		temp << "Out of Range";

	else
		temp << "X:" << ((uint32)x) << " Y:" << ((uint32)y);
	
	Coord->SetText(temp.String());
}
//----------------------------------------------------------------------
void MouseView::ClearClip()
{
	Size->SetText("- x -")	;
}
//----------------------------------------------------------------------
void MouseView::Clip1(float x, float y)
{
	Clip1x = (int) x;
	Clip1y = (int) y;
}
//----------------------------------------------------------------------
void MouseView::Clip2(float x, float y)
{
	BString temp;

	Clip2x = (int) x;
	Clip2y = (int) y;

	int lenghtX = abs(Clip1x - Clip2x - 1);
	int lenghtY = abs(Clip1y - Clip2y - 1);
	
	if(lenghtX > 4096 || lenghtY > 4096)
		temp << "- x -";
	else
		temp << lenghtX << " x " << lenghtY;

	Size->SetText(temp.String());
}
//----------------------------------------------------------------------
