/*
 * Copyright 2024, Johan Wagenheim <johan@dospuntos.no>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef WINDOWTITLEGUARD_H
#define WINDOWTITLEGUARD_H

#include <Window.h>

class WindowTitleGuard {
public:
	WindowTitleGuard(BWindow* window, BString tempTitle)
		:
		fWindow(window),
		fOriginalTitle(window->Title())
	{
		fWindow->SetTitle(tempTitle);
	}

	~WindowTitleGuard() { fWindow->SetTitle(fOriginalTitle); }

private:
	BWindow* fWindow;
	BString fOriginalTitle;
};

#endif // _H
