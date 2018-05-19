#include "main.h"
//--------------------------------------------------------------
int main()
{
	Resizer* programme; 
	programme = new Resizer;
	programme->Run();  
	delete programme;
	return 0;
}
//--------------------------------------------------------------
Resizer::Resizer() : BApplication("application/x-vnd.TAResizer")
{
	Fenetre = new MainWindow(); //fenetre ou est l'image
	Option = new OptionWindow(); //fenetre d'option
	Mouse = new MouseWindow(); //fenetre de la souris
	Fenetre->Show();
	Option->Show();
	Mouse->Run();
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
       	Fenetre->PostMessage(&aMessage, Fenetre->Main); // Post the message via the window
    }
}
