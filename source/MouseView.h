/*
 * Copyright 1999-2025. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *	1999-2000 Jonathan Villemure
 *
 */
#ifndef MOUSEVIEW_H
#define MOUSEVIEW_H

#include <StringView.h>

class MouseView : public BView {
public:
	MouseView();

	void ShowCoord(float x, float y);
	void ClearClip();
	void Clip1(float x, float y);
	void Clip2(float x, float y);

private:
	BStringView* fCoord;
	BStringView* fSize;
	int Clip1x, Clip1y, Clip2x, Clip2y;
};

#endif
