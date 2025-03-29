#include "main.h"

//--------------------------------------------------------------
int main()
{
	Resizer* app;
	app = new Resizer();
	app->Run();
	delete app;
	return 0;
}
//--------------------------------------------------------------
Resizer::Resizer() : BApplication("application/x-vnd.TAResizer")
{
	fMainWin = new MainWindow();
	fOptionWin = new OptionWindow();
	fMouseWin = new MouseWindow();

	fOptionWin->UpdateSizeLimits();
	int32 x = fOptionWin->Frame().RightTop().x + 10; //for some reason the x coordinate is 10px off
	int32 y = fOptionWin->Frame().RightTop().y;
	fMainWin->MoveTo(x, y);

	fMainWin->Show();
	fOptionWin->Show();
	fMouseWin->Run();
}
//--------------------------------------------------------------
void Resizer::RefsReceived(BMessage *message)
{
    entry_ref ref;
	// get the ref from the message
    if(message->FindRef("refs", &ref) == B_OK)
    {
        BMessage aMessage(B_SIMPLE_DATA); // Make a new message
        aMessage.AddRef( "refs", &ref ); // Copy the ref into it
       	fMainWin->PostMessage(&aMessage, fMainWin->fMainView); // Post the message via the window
    }
}
