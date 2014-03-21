#ifndef GEIMAPP_H
#define GEIMAPP_H

#include <wx/app.h>

class GEIMApp : public wxApp
{
	public:
		virtual bool OnInit();
		virtual int OnExit();
};

DECLARE_APP(GEIMApp)

#endif // GEIMAPP_H
