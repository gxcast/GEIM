#include "GEIMFrame.h"
#include "AboutDlg.h"

//(*InternalHeaders(GEIMFrame)
//*)
#include <wx/wx.h>


//(*IdInit(GEIMFrame)
//*)
const long GEIMFrame::ID_PANEL_MAIN = wxNewId();
const long GEIMFrame::ID_STATUSBAR_MAIN = wxNewId();
// tools buttons
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMIN = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMOUT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMRECT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMFIT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMACTUAL = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_MOVE = wxNewId();


BEGIN_EVENT_TABLE(GEIMFrame,wxFrame)
    //(*EventTable(GEIMFrame)
    //*)
END_EVENT_TABLE()

GEIMFrame::GEIMFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(GEIMFrame)
    //*)

    // create frame
    Create(parent, wxID_ANY, _("GEIM_Dev"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));

    // Set title bar icon
    SetIcon( wxIcon( _T("skin/GEIM.png"), wxBITMAP_TYPE_PNG ) );

	// create menu bar
	wxMenu* pMenu = nullptr;
	wxMenuItem* pMenuItem = nullptr;
	m_pMenuBarMain = new wxMenuBar();
	pMenu = new wxMenu();
	pMenuItem = new wxMenuItem(pMenu, wxID_OPEN, _("&Open\tCtrl-O"), _("Open Project"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, wxID_SAVE, _("&Save\tCtrl-S"), _("Save"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, wxID_CLOSE, _("&Close\tCtrl-W"), _("Close Project"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenu->AppendSeparator();
	pMenuItem = new wxMenuItem(pMenu, wxID_EXIT, _("&Exit\tAlt-F4"), _("Exit Program"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	m_pMenuBarMain->Append(pMenu, _("&File"));
	pMenu = new wxMenu();
	pMenuItem = new wxMenuItem(pMenu, wxID_ABOUT, _("&About\tF1"), _("About GEIM"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	m_pMenuBarMain->Append(pMenu, _("Help"));
	SetMenuBar(m_pMenuBarMain);

	// create status bar
	m_pStatusBarMain = new wxStatusBar(this, ID_STATUSBAR_MAIN, 0, _T("ID_STATUSBAR_MAIN"));
	int __wxStatusBarWidths_1[3] = { -1, 20, 20 };
	int __wxStatusBarStyles_1[3] = { wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL };
	m_pStatusBarMain->SetFieldsCount(3,__wxStatusBarWidths_1);
	m_pStatusBarMain->SetStatusStyles(3,__wxStatusBarStyles_1);
	m_pStatusBarMain->SetStatusText( _("Ready"), 0 );
	SetStatusBar(m_pStatusBarMain);

    // client area
    wxBoxSizer* pBoxSizerTop = new wxBoxSizer(wxHORIZONTAL);
    {
        // create main panel
        m_pPanelMain = new wxPanel(this, ID_PANEL_MAIN, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_MAIN"));
        m_pBoxSizerMain = new wxBoxSizer(wxVERTICAL);
        {
            // tools buttons
            wxSizer* pBoxSizerTools = new wxBoxSizer(wxHORIZONTAL);
            {
                wxBitmapButton* pBmpBtn = nullptr;

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_ZOOMIN, wxBitmap(_T("./skin/zoomin.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("ZoomIn Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_ZOOMOUT, wxBitmap(_T("./skin/zoomout.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("ZoomOut Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_ZOOMRECT, wxBitmap(_T("./skin/zoomrect.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("Zoom Sel Rect Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_ZOOMFIT, wxBitmap(_T("./skin/zoomfit.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("Fit Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_ZOOMACTUAL, wxBitmap(_T("./skin/zoomactual.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("Actual Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

                pBoxSizerTools->AddSpacer(2);

                pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_IMG_MOVE, wxBitmap(_T("./skin/imgmove.png"), wxBITMAP_TYPE_PNG));
                pBmpBtn->SetToolTip(_("Move Image"));
                pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
            }
            m_pBoxSizerMain->Add(pBoxSizerTools, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

            // create image display panel
            wxSizer* pBoxSizerImg = new wxBoxSizer(wxHORIZONTAL);
            {
                m_pImgPanel = new ImagePanel(m_pPanelMain, wxID_ANY, wxDefaultPosition, wxSize(700, 500) );
                pBoxSizerImg->Add(m_pImgPanel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
            }
            m_pBoxSizerMain->Add(pBoxSizerImg, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        }
        m_pPanelMain->SetSizer(m_pBoxSizerMain);

        m_pBoxSizerMain->Fit(m_pPanelMain);
        m_pBoxSizerMain->SetSizeHints(m_pPanelMain);
        pBoxSizerTop->Add(m_pPanelMain, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    }
    SetSizer(pBoxSizerTop);

    pBoxSizerTop->Fit(this);
    pBoxSizerTop->SetSizeHints(this);
    Center();

    // Event process
    Connect(wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GEIMFrame::OnOpen);
    Connect(wxID_EXIT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&GEIMFrame::OnQuit);
    Connect(wxID_ABOUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&GEIMFrame::OnAbout);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&GEIMFrame::OnClose);
    // image operate
    Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomIN);
    Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomOut);
    Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomRect);
    Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomFit);
    Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomActual);
    Connect(ID_BMPBTN_IMG_MOVE, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnImgMove);
}

GEIMFrame::~GEIMFrame()
{
    //(*Destroy(GEIMFrame)
    //*)
}

////////////////////////////////////////////////////////////////////////////////////////
void GEIMFrame::InitFrame()
{
}

////////////////////////////////////////////////////////////////////////////////////////
void GEIMFrame::OnOpen(wxCommandEvent& event)
{
    int iRet = -1;
    // if already open a image, confirm new open
    if (m_imgA.IsOk())
    {
        iRet = wxMessageBox(_("A picture is already open, OK to open a new picture?"),
                                 _("Confirm"),
                                 wxYES_NO|wxICON_QUESTION|wxYES_DEFAULT|wxCENTER,
                                 this);
        if (iRet == wxNO)
            return;
    }

    // choose the image file
    wxFileDialog dlgFile(this, _("Choice Picture"),
                         _T(""), _T(""),
                         _T("All File|*.*|Bitmap|*.bmp|JPEG|*.jpg;*.jpeg|PNG|*.png|TIFF|*.tif;*.tiff"),
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if( dlgFile.ShowModal() != wxID_OK )
        return;

    // destroy old image
    if (m_imgA.IsOk())
        m_imgA.Destroy();

    // open new image
    wxLogNull logNull;
    m_imgA.LoadFile( dlgFile.GetPath() );

    // tall image panel the new iamge
    m_pImgPanel->SetImg( m_imgA );
}

void GEIMFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void GEIMFrame::OnAbout(wxCommandEvent& event)
{
    AboutDlg about(this);
    about.ShowModal();
}

void GEIMFrame::OnClose(wxCloseEvent& event)
{
    if ( event.CanVeto() )
    {
        int iDlg = wxMessageBox( _("Are you sure to close the program?"), _("Confirm"), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT | wxCENTRE );
        if ( iDlg != wxYES )
        {
            event.Veto();
            return;
        }
    }
    Destroy();
}

void GEIMFrame::OnZoomIN(wxCommandEvent& event)
{
    m_pImgPanel->ImgZoomIn();
}

void GEIMFrame::OnZoomOut(wxCommandEvent& event)
{
    m_pImgPanel->ImgZoomOut();
}

void GEIMFrame::OnZoomRect(wxCommandEvent& event)
{
    m_pImgPanel->ImgZoomRect();
}

void GEIMFrame::OnZoomFit(wxCommandEvent& event)
{
    m_pImgPanel->ImgZoomFit();
}

void GEIMFrame::OnZoomActual(wxCommandEvent& event)
{
    m_pImgPanel->ImgZoomActual();
}

void GEIMFrame::OnImgMove(wxCommandEvent& event)
{
    m_pImgPanel->ImgMove();
}
