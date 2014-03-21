#include "AboutDlg.h"
#include <wx/wx.h>

//(*InternalHeaders(AboutDlg)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(AboutDlg)
//*)
const long AboutDlg::ID_HTMWND = wxNewId();

BEGIN_EVENT_TABLE(AboutDlg,wxDialog)
    //(*EventTable(AboutDlg)
    //*)
END_EVENT_TABLE()

AboutDlg::AboutDlg(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
    : m_pHtmWnd( NULL )
{
    //(*Initialize(AboutDlg)
    //*)
    Create(parent, id, _("About GEIM..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDLG_UNIT(parent, wxSize(200,138)));
    Center();
    m_pHtmWnd = new wxHtmlWindow( this,
            ID_HTMWND,
            wxDefaultPosition, wxDefaultSize,
            wxHW_SCROLLBAR_NEVER|wxHW_NO_SELECTION,
            _T("HtmWnd") );
    m_pHtmWnd->LoadPage( _T("file:skin/about.htm") );
    Connect(ID_HTMWND, wxEVT_HTML_CELL_CLICKED, (wxObjectEventFunction)&AboutDlg::OnHtmlCellClicked);
}

AboutDlg::~AboutDlg()
{
    //(*Destroy(AboutDlg)
    //*)
}

void AboutDlg::OnHtmlCellClicked(wxHtmlCellEvent& event)
{
    wxHtmlCell* pCell = event.GetCell();
    if ( pCell == NULL || pCell->GetId().IsEmpty() )
        return;
    if ( pCell->GetId() == _T("btnClose") )
        EndModal(wxID_CANCEL);
}
