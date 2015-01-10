#include "GEIMApp.h"
#include "GEIMFrame.h"

#include <wx/wx.h>
#include <wx/filename.h>


IMPLEMENT_APP(GEIMApp);

bool GEIMApp::OnInit()
{
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if (m_pLocale == nullptr)
	{
		m_pLocale = new wxLocale();
		if (m_pLocale->Init())
		{
			m_pLocale->AddCatalogLookupPathPrefix(_T("./locale"));
			m_pLocale->AddCatalog(_T("GEIM"));
		}
	}
	if (wxsOK)
	{
		GEIMFrame* Frame = new GEIMFrame(0);
		Frame->Show();
		SetTopWindow(Frame);
	}
	return wxsOK;
}

int GEIMApp::OnExit()
{
	if (m_pLocale != nullptr)
	{
		delete m_pLocale;
		m_pLocale = nullptr;
	}
	return 0;
}
