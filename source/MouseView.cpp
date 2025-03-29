#include "MouseView.h"
#include <Application.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <stdlib.h>
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MouseView"

//----------------------------------------------------------------------
MouseView::MouseView()
	: BView("MouseView", B_WILL_DRAW)
{
	fCoord = new BStringView("Coord", "X:0 Y:0");
	fSize = new BStringView("Size", "- x -");

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
			.Add(fCoord)
			.Add(fSize)
		.End();

}
//----------------------------------------------------------------------
void MouseView::ShowCoord(float x, float y)
{
	BString temp;
	if(x > 4096 || x < 0 || y > 4096 || y < 0)
		temp << B_TRANSLATE("Out of range");

	else
		temp << "X:" << ((uint32)x) << " Y:" << ((uint32)y);

	fCoord->SetText(temp.String());
}
//----------------------------------------------------------------------
void MouseView::ClearClip()
{
	fSize->SetText("- x -")	;
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

	fSize->SetText(temp.String());
}
//----------------------------------------------------------------------
