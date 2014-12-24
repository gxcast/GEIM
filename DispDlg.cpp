#include "DispDlg.h"

// tools buttons
const long DispDlg::ID_BMPBTN_IMG_ZOOMIN = wxNewId();
const long DispDlg::ID_BMPBTN_IMG_ZOOMOUT = wxNewId();
const long DispDlg::ID_BMPBTN_IMG_ZOOMRECT = wxNewId();
const long DispDlg::ID_BMPBTN_IMG_ZOOMFIT = wxNewId();
const long DispDlg::ID_BMPBTN_IMG_ZOOMACTUAL = wxNewId();
const long DispDlg::ID_BMPBTN_IMG_MOVE = wxNewId();

IMPLEMENT_CLASS(DispDlg, wxDialog)

DispDlg::DispDlg()
{
}

DispDlg::DispDlg(wxWindow* parent,
                 wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& sz)
{
	Create(parent, id, title, pos, sz);
}

DispDlg::~DispDlg()
{
	ReleaseImg();
}

bool DispDlg::Create(wxWindow* parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos,
                     const wxSize& sz)
{
	bool bRet = false;
	// set extra styles
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
	// create
	bRet = wxDialog::Create(parent, id, title, pos, sz, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _("ImageDisplay"));
	if (!bRet)
		return bRet;
	// create controls
	bRet = CreateControl();
	if (!bRet)
		return bRet;
	// event map
	bRet = DyEventMap();
	if (!bRet)
		return bRet;
	// centre dialog on the parente
	Centre();

	return bRet;
}

bool DispDlg::SetImage(wxImage* pImg)
{
	if (pImg == nullptr || !pImg->Ok())
		return false;

	if (m_pImgDisp == nullptr)
		m_pImgDisp = new wxImage();
	else if (m_pImgDisp->Ok())
	{
		m_pImgPanel->ReleaseImg();
		m_pImgDisp->Destroy();
	}
	*m_pImgDisp = pImg->Copy();
	m_pImgPanel->SetImg(*m_pImgDisp);
	return true;
}

bool DispDlg::SetImage(unsigned char *pData, int iW, int iH)
{
	if (pData == nullptr || iW <= 0 || iH <= 0)
		return false;

	if (m_pImgDisp == nullptr)
		m_pImgDisp = new wxImage(iW, iH);
	else if (!m_pImgDisp->IsOk())
		m_pImgDisp->Create(iW, iH);
	else if (m_pImgDisp->IsOk() && (m_pImgDisp->GetWidth() != iW || m_pImgDisp->GetHeight() != iH))
	{
		m_pImgPanel->ReleaseImg();
		m_pImgDisp->Destroy();
		m_pImgDisp->Create(iW, iH);
	}
	unsigned char *pCache = m_pImgDisp->GetData();
	int iN = iW*iH*3;
	memcpy(pCache, pData, (size_t)iN);
	m_pImgPanel->SetImg(*m_pImgDisp);

	return true;
}

bool DispDlg::ReleaseImg()
{
	if (m_pImgDisp != nullptr)
	{
		if (m_pImgDisp->IsOk())
		{
			m_pImgPanel->ReleaseImg();
			m_pImgDisp->Destroy();
		}
		delete m_pImgDisp;
		m_pImgDisp = nullptr;
	}
	return true;
}

/**< create controls and layout */
bool DispDlg::CreateControl()
{
	wxBoxSizer* pTopSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxBoxSizer* pTools = new wxBoxSizer(wxVERTICAL);
		{
			// buttons
			wxBitmapButton* pBmpBtn = nullptr;

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_ZOOMIN, wxBitmap(_T("./skin/zoomin.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("ZoomIn Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_ZOOMOUT, wxBitmap(_T("./skin/zoomout.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("ZoomOut Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_ZOOMRECT, wxBitmap(_T("./skin/zoomrect.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("Zoom Sel Rect Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_ZOOMFIT, wxBitmap(_T("./skin/zoomfit.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("Fit Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_ZOOMACTUAL, wxBitmap(_T("./skin/zoomactual.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("Actual Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);

			pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_IMG_MOVE, wxBitmap(_T("./skin/imgmove.png"), wxBITMAP_TYPE_PNG));
			pBmpBtn->SetToolTip(_("Move Image"));
			pTools->Add(pBmpBtn, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 2);
		}
		pTopSizer->Add(pTools, 0, wxLEFT|wxALIGN_CENTER, 3);

		// image panel
		m_pImgPanel = new ImagePanel(this, wxID_ANY, wxDefaultPosition, wxSize(800, 500) );
		pTopSizer->Add(m_pImgPanel, 1, wxALL|wxALIGN_CENTER|wxEXPAND, 3);
	}
	SetSizer(pTopSizer);
	pTopSizer->Fit(this);
	pTopSizer->SetSizeHints(this);

	return true;
}

/**< dynamic event map */
bool DispDlg::DyEventMap()
{
	// image operate bottons
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnZoomIN);
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnZoomOut);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnZoomRect);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnZoomFit);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnZoomActual);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_BUTTON, (wxObjectEventFunction)&DispDlg::OnImgMove);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&DispDlg::OnBtnsUpdate);

	return true;
}

/**< zoom in image */
void DispDlg::OnZoomIN(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomIn();
}

/**< zoom out image */
void DispDlg::OnZoomOut(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomOut();
}

/**< zoom image depend sel rect */
void DispDlg::OnZoomRect(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomRect();
}

/**< zoom image fit the wnd */
void DispDlg::OnZoomFit(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomFit();
}

/**< display image in it's actual size */
void DispDlg::OnZoomActual(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomActual();
}

/**< move iamge to diaplay different region */
void DispDlg::OnImgMove(wxCommandEvent& event)
{
	m_pImgPanel->ImgMove();
}

/**< update tool btns' state */
void DispDlg::OnBtnsUpdate(wxUpdateUIEvent& event)
{
	if (m_pImgDisp == nullptr)
	{
		event.Enable(false);
		return;
	}

	long id = event.GetId();
	if (id == ID_BMPBTN_IMG_ZOOMIN)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZIN, event);
	else if (id == ID_BMPBTN_IMG_ZOOMOUT)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZOUT, event);
	else if (id == ID_BMPBTN_IMG_ZOOMRECT)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZRECT, event);
	else if (id == ID_BMPBTN_IMG_ZOOMFIT)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZFIT, event);
	else if (id == ID_BMPBTN_IMG_ZOOMACTUAL)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_ZACTUAL, event);
	else if (id == ID_BMPBTN_IMG_MOVE)
		m_pImgPanel->UpdateUI(IMGPL_CMD::IMG_MOVE, event);
}
