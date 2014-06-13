#pragma once
#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <wx/dialog.h>

#include <wx/html/htmlwin.h>

class AboutDlg: public wxDialog
{
	public:

		AboutDlg(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AboutDlg();

	private:

		void OnHtmlCellClicked(wxHtmlCellEvent& event);

		static const long ID_HTMWND;

        wxHtmlWindow* m_pHtmWnd;

		DECLARE_EVENT_TABLE()
};

#endif
