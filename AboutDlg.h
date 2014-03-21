#ifndef ABOUTDLG_H
#define ABOUTDLG_H

//(*Headers(AboutDlg)
#include <wx/dialog.h>
//*)
#include <wx/html/htmlwin.h>

class AboutDlg: public wxDialog
{
	public:

		AboutDlg(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AboutDlg();

	private:

		//(*Handlers(AboutDlg)
		//*)
		void OnHtmlCellClicked(wxHtmlCellEvent& event);

		//(*Identifiers(AboutDlg)
		//*)
		static const long ID_HTMWND;

		//(*Declarations(AboutDlg)
        wxHtmlWindow* m_pHtmWnd;
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
