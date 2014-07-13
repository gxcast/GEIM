#include "AboutDlg.h"
#include <wx/wx.h>

#include <wx/intl.h>
#include <wx/string.h>

const long AboutDlg::ID_HTMWND = wxNewId();

BEGIN_EVENT_TABLE(AboutDlg,wxDialog)
END_EVENT_TABLE()

AboutDlg::AboutDlg(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
    : m_pHtmWnd( NULL )
{
    Create(parent, id, _("About GEIM..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("About GEIM"));
    wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    {
		m_pHtmWnd = new wxHtmlWindow( this,
		ID_HTMWND,
		wxDefaultPosition, wxSize(480,310),
		wxHW_SCROLLBAR_NEVER|wxHW_NO_SELECTION,
		_T("HtmWnd") );
		m_pHtmWnd->LoadPage( _T("file:skin/about.htm") );
		pSizer->Add(m_pHtmWnd, 1, wxALL|wxALIGN_CENTER, 0);
	}
	SetSizer(pSizer);
	pSizer->Fit(this);
	pSizer->SetSizeHints(this);
    Center();

    Connect(ID_HTMWND, wxEVT_HTML_CELL_CLICKED, (wxObjectEventFunction)&AboutDlg::OnHtmlCellClicked);
}

AboutDlg::~AboutDlg()
{
}

void AboutDlg::OnHtmlCellClicked(wxHtmlCellEvent& event)
{
    wxHtmlCell* pCell = event.GetCell();
    if ( pCell == NULL || pCell->GetId().IsEmpty() )
        return;
    if ( pCell->GetId() == _T("btnClose") )
        EndModal(wxID_CANCEL);
}
