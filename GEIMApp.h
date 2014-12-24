#ifndef GEIMAPP_H
#define GEIMAPP_H

#include <wx/app.h>

class wxLocale;
class GEIMApp : public wxApp
{
	wxLocale *m_pLocale = nullptr;
	public:
		virtual bool OnInit();
		virtual int OnExit();
};

DECLARE_APP(GEIMApp)

#endif // GEIMAPP_H
