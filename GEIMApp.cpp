#include "GEIMApp.h"
#include "GEIMFrame.h"

#include <wx/wx.h>


IMPLEMENT_APP(GEIMApp);

bool GEIMApp::OnInit()
{
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
