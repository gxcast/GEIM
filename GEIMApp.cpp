#include "GEIMApp.h"
#include "GEIMFrame.h"

//(*AppHeaders
//*)
#include <wx/wx.h>


IMPLEMENT_APP(GEIMApp);

bool GEIMApp::OnInit()
{
	//(*AppInitialize
	//*)
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if ( wxsOK )
	{
		GEIMFrame* Frame = new GEIMFrame(0);
		Frame->Show();
		SetTopWindow(Frame);
	}
	return wxsOK;
}

int GEIMApp::OnExit()
{
	return 0;
}
