#include "GEIMFrame.h"
#include "AboutDlg.h"
#include "SpotDtDlg.h"

//(*InternalHeaders(GEIMFrame)
//*)
#include <wx/wx.h>


//(*IdInit(GEIMFrame)
//*)
const long GEIMFrame::ID_PANEL_MAIN = wxNewId();
const long GEIMFrame::ID_STATUSBAR_MAIN = wxNewId();
const long GEIMFrame::ID_CMD_DT = wxNewId();
const long GEIMFrame::ID_CMD_MT = wxNewId();
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
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_DT, _("&Detect"), _("Detect Spots"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_MT, _("&Match"), _("Match Spots"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	m_pMenuBarMain->Append(pMenu, _("&Proc"));
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
		m_pPanelMain = new wxPanel(this, ID_PANEL_MAIN, wxDefaultPosition, wxSize(800, 600), wxTAB_TRAVERSAL, _T("ID_PANEL_MAIN"));
		m_pPanelMain->SetMinSize(wxSize(800, 600));
		m_pBoxSizerMain = new wxBoxSizer(wxVERTICAL);
		{
			// tools buttons
			wxBoxSizer* pBoxSizerTools = new wxBoxSizer(wxHORIZONTAL);
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
			m_pBoxSizerImg = new wxBoxSizer(wxHORIZONTAL);
			{
				ImagePanel* pImgPanel = new ImagePanel(m_pPanelMain, wxNewId());
				pImgPanel->SetMinSize(wxSize(250, 200));
				m_aryPanels.Add(pImgPanel);
				m_pBoxSizerImg->Add(pImgPanel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
				Connect(pImgPanel->GetId(), wxEVT_IMGPL, (wxObjectEventFunction)&GEIMFrame::OnImgplNtfy);
			}
			m_pBoxSizerMain->Add(m_pBoxSizerImg, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
		}
		m_pPanelMain->SetSizer(m_pBoxSizerMain);

		m_pBoxSizerMain->Fit(m_pPanelMain);
		//m_pBoxSizerMain->SetSizeHints(m_pPanelMain);
		pBoxSizerTop->Add(m_pPanelMain, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	}
	SetSizer(pBoxSizerTop);

	pBoxSizerTop->Fit(this);
	pBoxSizerTop->SetSizeHints(this);
	Center();

	// Event process
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&GEIMFrame::OnClose);
	// menu or tool-button command
	Connect(wxID_OPEN, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnFileOpen);
	Connect(wxID_CLOSE, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnFileClose);
	Connect(wxID_CLOSE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnFileCloseUpdate);
	Connect(ID_CMD_DT, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnDt);
	Connect(ID_CMD_DT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnDtUpdate);
	Connect(ID_CMD_MT, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnMt);
	Connect(ID_CMD_MT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnMtUpdate);
	Connect(wxID_EXIT,wxEVT_MENU,(wxObjectEventFunction)&GEIMFrame::OnQuit);
	Connect(wxID_ABOUT,wxEVT_MENU,(wxObjectEventFunction)&GEIMFrame::OnAbout);
	// image operate
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomIN);
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomOut);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomRect);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomFit);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnZoomActual);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnImgMove);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
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
/**< invoke when window will close */
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

	// release resource, send file close event
	wxCommandEvent evt(wxEVT_MENU, wxID_CLOSE);
	evt.SetEventObject(this);
	GetEventHandler()->ProcessEvent(evt);

	Destroy();
}

/**< invoke when open a file */
void GEIMFrame::OnFileOpen(wxCommandEvent& event)
{
	int iRet = -1;
	// if already open a image, confirm new open
	size_t nNum = m_aryImgs.Count();
	if (nNum > 0)
	{
		iRet = wxMessageBox(_("A group of images are already opened, SURE to open a new group of iamges?"),
		                    _("Confirm"),
		                    wxYES_NO|wxICON_QUESTION|wxYES_DEFAULT|wxCENTER,
		                    this);
		if (iRet != wxOK)
			return;
	}

	// choose images file
	wxFileDialog dlgFile(this, _("Choice Images"),
	                     _T(""), _T(""),
	                     _T("All Image File|*.bmp;*.jpg;*.jpeg;*.png;*.tif;*.tiff|Bitmap|*.bmp|JPEG|*.jpg;*.jpeg|PNG|*.png|TIFF|*.tif;*.tiff"),
	                     wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE);
	if( dlgFile.ShowModal() != wxID_OK )
		return;
	// files num
	wxArrayString aryFiles;
	dlgFile.GetPaths(aryFiles);
	nNum = aryFiles.Count();
	if (nNum != 2)
	{
		wxMessageBox(_("Files' number must be 2."),
		             _("Error"),
		             wxOK|wxICON_ERROR|wxCENTER,
		             this);
		return;
	}

	// destroy old image, send file close event
	wxCommandEvent evt(wxEVT_MENU, wxID_CLOSE);
	evt.SetEventObject(this);
	GetEventHandler()->ProcessEvent(evt);

	// new display image panel
	size_t nPanels = m_aryPanels.Count();
	if (nPanels < nNum)     // add new panel
	{
		for (size_t i = nPanels; i < nNum; ++i)
		{
			ImagePanel* pImgPanel = new ImagePanel(m_pPanelMain, wxNewId());
			pImgPanel->SetMinSize(wxSize(250, 200));
			m_aryPanels.Add(pImgPanel);
			m_pBoxSizerImg->Add(pImgPanel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
			Connect(pImgPanel->GetId(), wxEVT_IMGPL, (wxObjectEventFunction)&GEIMFrame::OnImgplNtfy);
		}
		nPanels = m_aryPanels.Count();
	}
	// panel dispaly status modify
	{
		size_t i = 0;
		for (; i < nNum; ++i)       // show the first few panels
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->Show();
		}
		for (; i < nPanels; ++i)    // hdie extra panels
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->Hide();
		}
	}
	// update layout
	m_pBoxSizerImg->Layout();
	//m_pBoxSizerImg->Fit(m_pPanelMain);

	// open new images
	for (size_t i = 0; i < nNum; ++i)
	{
		wxString& path = aryFiles.Item(i);
		wxFileName fnm(path);
		const wxString& nm = fnm.GetFullName();
		// new image
		wxImage* pImg = new wxImage(path);
		pImg->SetOption(wxIMAGE_OPTION_FILENAME, nm);
		m_aryImgs.Add(pImg);

		// display image
		wxImage* pImgDisp = new wxImage();
		*pImgDisp = pImg->Copy();
		m_aryImgsDisp.Add(pImgDisp);
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->SetImg(*pImgDisp);
	}
}

/**< command to close the opened file */
void GEIMFrame::OnFileClose(wxCommandEvent& event)
{
	// destroy origin images
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImg = static_cast<wxImage*>(m_aryImgs.Item(i));
		if (pImg == nullptr)
			continue;
		pImg->Destroy();
		delete pImg;
	}
	m_aryImgs.Empty();

	// detroy display images
	nNum = m_aryImgsDisp.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		// ui stop display image
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ReleaseImg();
		// hide image panels
		if (i > 0)
			pPanel->Hide();

		wxImage* pImg = static_cast<wxImage*>(m_aryImgsDisp.Item(i));
		if (pImg == nullptr)
			continue;
		pImg->Destroy();
		delete pImg;
	}
	m_aryImgsDisp.Empty();
	// update ui layout
	m_pBoxSizerImg->Layout();

	// detroy detected images
	nNum = m_aryImgsDt.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImg = static_cast<wxImage*>(m_aryImgsDt.Item(i));
		if (pImg == nullptr)
			continue;
		pImg->Destroy();
		delete pImg;
	}
	m_aryImgsDt.Empty();
}
void GEIMFrame::OnFileCloseUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	event.Enable(nNum > 0);
}

/**< detect spots */
void GEIMFrame::OnDt(wxCommandEvent& event)
{
	int iRet = 0;

	// destroy old dt images
	size_t nNum = m_aryImgsDt.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImgDt = static_cast<wxImage*>(m_aryImgsDt.Item(i));
		pImgDt->Destroy();
		delete pImgDt;
	}
	m_aryImgsDt.Empty();
	// new create a copy of images
	nNum = m_aryImgsDisp.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImgDisp = static_cast<wxImage*>(m_aryImgsDisp.Item(i));
		wxImage* pImgDt = new wxImage();
		*pImgDt = pImgDisp->Copy();
		m_aryImgsDt.Add(pImgDt);
	}

	// show detect dialog
	SpotDtDlg dlg(this, wxNewId());
	dlg.Init(&m_aryImgsDt);     // set images
	iRet = dlg.ShowModal();
	if (iRet != wxID_OK)
		return;

	// update dispaly iamges
	for (size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImgDisp = static_cast<wxImage*>(m_aryImgsDisp.Item(i));
		wxImage* pImgDt = static_cast<wxImage*>(m_aryImgsDt.Item(i));
		*pImgDisp = pImgDt->Copy();
	}
}
void GEIMFrame::OnDtUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	event.Enable(nNum > 0);
}

/**< match spots */
void GEIMFrame::OnMt(wxCommandEvent& event)
{

}
void GEIMFrame::OnMtUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	event.Enable(nNum > 0);
}

/**< command to exit the app */
void GEIMFrame::OnQuit(wxCommandEvent& event)
{
	Close();
}

/**< command to display app info */
void GEIMFrame::OnAbout(wxCommandEvent& event)
{
	AboutDlg about(this);
	about.ShowModal();
}

/**< zoom in image */
void GEIMFrame::OnZoomIN(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgZoomIn();
	}
}

/**< zoom out image */
void GEIMFrame::OnZoomOut(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgZoomOut();
	}
}

/**< zoom image depend sel rect */
void GEIMFrame::OnZoomRect(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgZoomRect();
	}
}

/**< zoom image fit the wnd */
void GEIMFrame::OnZoomFit(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgZoomFit();
	}
}

/**< display image in it's actual size */
void GEIMFrame::OnZoomActual(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgZoomActual();
	}
}

/**< move iamge to diaplay different region */
void GEIMFrame::OnImgMove(wxCommandEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->ImgMove();
	}
}

/**< update tool btns' state */
void GEIMFrame::OnBtnsUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	if (nNum > 0)
	{
		ImagePanel* pImgPanel = static_cast<ImagePanel*>(m_aryPanels.Item(0));
		wxASSERT_MSG(pImgPanel != nullptr, _T("Frame update ui, the first image is invalide."));

		long id = event.GetId();
		if (id == ID_BMPBTN_IMG_ZOOMIN)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZIN, event);
		else if (id == ID_BMPBTN_IMG_ZOOMOUT)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZOUT, event);
		else if (id == ID_BMPBTN_IMG_ZOOMRECT)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZRECT, event);
		else if (id == ID_BMPBTN_IMG_ZOOMFIT)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZFIT, event);
		else if (id == ID_BMPBTN_IMG_ZOOMACTUAL)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZACTUAL, event);
		else if (id == ID_BMPBTN_IMG_MOVE)
			pImgPanel->UpdateUI(IMGPL_CMD::IMG_MOVE, event);
	}
}

/**< invoke when these iamge panels notify */
void GEIMFrame::OnImgplNtfy(wxImgplEvent& event)
{
	int id = event.GetId();
	IMGPL_CMD cmd = event.GetCMD();
	void* pParam = event.GetParam();

	size_t nNum = m_aryImgsDisp.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		// pass over itself
		if (pPanel->GetId() == id)
			continue;

		// execu notify
		switch(cmd)
		{
		case IMGPL_CMD::IMG_ZRECT:
			{
				wxRect* prcSel = static_cast<wxRect*>(pParam);
				wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get event param failed."));
				pPanel->ImgZoomRect(*prcSel);
			}
			break;
		case IMGPL_CMD::IMG_MOVE:
			{
				wxSize* pszMv = static_cast<wxSize*>(pParam);
				wxASSERT_MSG(pszMv != nullptr, _T("EVT_IMGPL get event param failed."));
				pPanel->ImgMove(*pszMv);
			}
			break;
		default:
			break;
		}
	}
}
