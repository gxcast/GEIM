#include "GEIMFrame.h"

#include <wx/wx.h>

#include "AboutDlg.h"
#include "SpotDtDlg.h"
#include "EffectPar.h"
#include "SpotDtThread.h"
#include "CharactVect.h"
#include "Graying.h"


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
END_EVENT_TABLE()

GEIMFrame::GEIMFrame(wxWindow* parent,wxWindowID id)
{
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
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_DT, _("&Detect\tCtrl-D"), _("Detect Spots"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_MT, _("&Match\tCtrl-M"), _("Match Spots"), wxITEM_NORMAL);
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
	Connect(wxID_OPEN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnFileOpenUpdate);
	Connect(wxID_CLOSE, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnFileClose);
	Connect(wxID_CLOSE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnFileCloseUpdate);
	Connect(ID_CMD_DT, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnDt);
	Connect(ID_CMD_DT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnDtUpdate);
	Connect(ID_CMD_MT, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnMt);
	Connect(ID_CMD_MT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnMtUpdate);
	Connect(wxID_EXIT, wxEVT_MENU,(wxObjectEventFunction)&GEIMFrame::OnQuit);
	Connect(wxID_EXIT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnQuitUpdate);
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
	// thread event
	Connect(SpotDtThread::ID, wxEVT_THREAD, (wxObjectEventFunction)&GEIMFrame::OnThreadDt);
}

GEIMFrame::~GEIMFrame()
{
}

/**< detroy the mt param */
bool GEIMFrame::DestroyMTParam(ST_MTPARAM* pstParm)
{
	if (pstParm == nullptr)
		return true;

	// origin image
	if (pstParm->pImg != nullptr)
	{
		delete [] pstParm->pImg;
		pstParm->pImg = nullptr;
	}

	// detect result maybe come from result list
	pstParm->pData = nullptr;

	// attrib vector
	if (pstParm->pvtAttr != nullptr)
	{
		for (auto it = pstParm->pvtAttr->begin(); it != pstParm->pvtAttr->end(); ++it)
		{
			ST_SPOT_ATTR& attr = *it;

			// detect result maybe come from result list
			attr.pNode = nullptr;
			// release charact
			if (attr.pCrt != nullptr)
			{
				delete attr.pCrt;
				attr.pCrt = nullptr;
			}
		}
		if (!pstParm->pvtAttr->empty())
			pstParm->pvtAttr->clear();

		delete pstParm->pvtAttr;
		pstParm->pvtAttr = nullptr;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
/**< refresh all the dispaly image */
bool GEIMFrame::RefreshImgs()
{
	size_t nNum = m_aryImgs.Count();
	for (size_t i = 0; i < nNum; ++i)
	{
		ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
		pPanel->Refresh(false);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
/**< invoke when window will close */
void GEIMFrame::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto() && m_aryImgs.Count() > 0)
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
		iRet = wxMessageBox(_("A group of images are already opened,\r\n SURE to open a new group of iamges?"),
		                    _("Confirm"),
		                    wxYES_NO|wxICON_QUESTION|wxYES_DEFAULT|wxCENTER,
		                    this);
		if (iRet != wxYES)
			return;
	}

	// choose images file
	wxFileDialog dlgFile(this, _("Choice Images"),
	                     _T(""), _T(""),
	                     _T("All Image File|*.bmp;*.jpg;*.jpeg;*.png;*.tif;*.tiff|Bitmap|*.bmp|JPEG|*.jpg;*.jpeg|PNG|*.png|TIFF|*.tif;*.tiff"),
	                     wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE|wxFD_PREVIEW);
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
void GEIMFrame::OnFileOpenUpdate(wxUpdateUIEvent& event)
{
	event.Enable(m_pBusy == nullptr);
}

/**< command to close the opened file */
void GEIMFrame::OnFileClose(wxCommandEvent& event)
{
	// realse match result
	// realse detect result
	SpotDtThread::DestroyDtResult(&m_lsDtResult);

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
}
void GEIMFrame::OnFileCloseUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	event.Enable(nNum > 0 && m_pBusy == nullptr);
}

/**< detect spots */
void GEIMFrame::OnDt(wxCommandEvent& event)
{
	int iRet = 0;

	// show detect dialog, get the detect parameter
	SpotDtDlg dlg(this, wxNewId());
	dlg.Init(&m_aryImgs);     // set dt images
	iRet = dlg.ShowModal();
	if (iRet != wxID_OK)
		return;

	// detect the spot in batch, at the same, display the result
	ST_DTPARAM& stDtParam = dlg.DtParam();
	SpotDtThread* pThd = new SpotDtThread(this, stDtParam, m_aryImgs, m_aryImgsDisp, m_lsDtResult);
	if (pThd->Run() != wxTHREAD_NO_ERROR)	// pThd delete itself auto
	{
		wxMessageBox(_("Create dtect thread failed."), _("Information"), wxOK|wxICON_INFORMATION|wxCENTER, this);
		return;
	}
	// busy indicator
	m_pBusy = new wxBusyInfo(_("Please wait, batch detecting..."), this);
}
void GEIMFrame::OnDtUpdate(wxUpdateUIEvent& event)
{
	size_t nNum = m_aryImgs.Count();
	event.Enable(nNum > 0 && m_pBusy == nullptr);
}

/**< match spots */
void GEIMFrame::OnMt(wxCommandEvent& event)
{
	ST_MTPARAM stParamA;
    ST_MTPARAM stParamB;
    ST_MTRESULT stMtRst;

    auto FillParam = [this](ST_MTPARAM& stParam, int id) -> bool {
		wxImage* pImg = static_cast<wxImage*>(m_aryImgs.Item(id));
		unsigned char* pTmp = pImg->GetData();
		auto it = m_lsDtResult.begin();
		for (int i = 0; i < id; ++i)
			++it;
		ST_DTRESULT& rst = *it;

		stParam.iW = pImg->GetWidth();
		stParam.iH = pImg->GetHeight();
		stParam.iWb = stParam.iW*3;
		stParam.iN = stParam.iW*stParam.iH;

		stParam.pImg = new ST_RGB[stParam.iN];
		memcpy(stParam.pImg, pTmp, stParam.iN*3);

		stParam.pData = rst.pData;

		size_t nC = rst.pLs->size();
		stParam.pvtAttr = new VT_ATTR(nC);
		nC = 0u;
		for(auto ir = rst.pLs->begin(); ir != rst.pLs->end(); ++ir)
		{
			ST_SPOT_NODE& stNode = *ir;
			ST_SPOT_ATTR& attr = stParam.pvtAttr->at(nC++);
			attr.pNode = &stNode;
			attr.pCrt = new ST_SPOT_CHARACT;
		}
		wxASSERT_MSG(nC == rst.pLs->size(), _T("Fill match param's pNode failed."));

		return true;
    };
    FillParam(stParamA, 0);
    FillParam(stParamB, 1);

    // match
    CharactVect cvt;
    if (cvt.CVMain(std::make_pair(stParamA, stParamB), &stMtRst))
	{
		// get the result
		auto GetRst = [this](ST_MTPARAM& stParam, int id) -> bool {
			wxImage* pImg = static_cast<wxImage*>(m_aryImgsDisp.Item(id));
			PST_RGB pDes = (PST_RGB)pImg->GetData();
			int iW = stParam.iW;
			int iH = stParam.iH;
			int iN = stParam.iN;
			// copy the image
			memcpy(pDes, stParam.pImg, iN*3);
			// draw spot's character
			for (auto it = stParam.pvtAttr->begin(); it != stParam.pvtAttr->end(); ++it)
			{
				if (it->bInvalid)
					continue;
				ST_SPOT_NODE& spot = *(it->pNode);
				ST_SPOT_CHARACT& crt = *(it->pCrt);
				// 蛋白点座标
				int x = spot.x;
				int y = spot.y;
				// character
				ST_RGB clr;
				Graying::ColorMap(crt.plump, &clr);
				for (int j = -3; j <= 3; ++j)
				{
					if (y+j < 0 || y+j >= iH)
						continue;
					PST_RGB pLine = pDes + (y+j)*iW;
					for (int i = -3; i <=3; ++i)
					{
						if (x+i < 0 || x+i >= iW)
							continue;
						PST_RGB pPix = pLine + (x+i);
						*pPix = clr;
					}
				}
			}

			return true;
		};
		GetRst(stParamA, 0);
		GetRst(stParamB, 1);

		// update ui
		RefreshImgs();
	}

    // release param
    GEIMFrame::DestroyMTParam(&stParamA);
    GEIMFrame::DestroyMTParam(&stParamB);
}
void GEIMFrame::OnMtUpdate(wxUpdateUIEvent& event)
{
	bool bEn = true;
	size_t nNum = m_aryImgs.Count();
	bEn = (nNum > 0);
	bEn = bEn && (m_pBusy == nullptr);
	bEn = bEn && (m_lsDtResult.size() >= 2);
	event.Enable(bEn);
}

/**< command to exit the app */
void GEIMFrame::OnQuit(wxCommandEvent& event)
{
	Close();
}
void GEIMFrame::OnQuitUpdate(wxUpdateUIEvent& event)
{
	event.Enable(m_pBusy == nullptr);
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
	long id = event.GetId();
	size_t nNum = m_aryImgs.Count();
	if (nNum <= 0)
	{
		event.Enable(false);
		return;
	}
	ImagePanel* pImgPanel = static_cast<ImagePanel*>(m_aryPanels.Item(0));
	wxASSERT_MSG(pImgPanel != nullptr, _T("Frame update ui, the first image is invalide."));

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

/**< invoke when these iamge panels notify */
void GEIMFrame::OnImgplNtfy(wxImgplEvent& event)
{
	int id = event.GetId();
	IMGPL_CMD cmd = event.GetCMD();
	void* pParam = event.GetParam();

	size_t nNum = m_aryImgs.Count();
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
			wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get IMG_ZRECT event param failed."));
			pPanel->ImgZoomRect(*prcSel);
		}
		break;
		case IMGPL_CMD::IMG_MOVE:
		{
			wxSize* pszMv = static_cast<wxSize*>(pParam);
			wxASSERT_MSG(pszMv != nullptr, _T("EVT_IMGPL get IMG_MOVE event param failed."));
			pPanel->ImgMove(*pszMv);
		}
		break;
		default:
			break;
		}
	}
}

/**< finish batch detect event*/
void GEIMFrame::OnThreadDt(wxThreadEvent& event)
{
	bool bRet = (event.GetInt() == 1);

	// stop indicator
	if (m_pBusy != nullptr)
	{
		delete m_pBusy;
		m_pBusy = nullptr;
	}

	// notice
	if (!bRet)
		wxMessageBox(_("Detect failed!"), _("Information"), wxOK|wxICON_INFORMATION|wxCENTER, this);
	else
		// update ui
		RefreshImgs();
}

/**< finish match event*/
void GEIMFrame::OnThreadMt(wxThreadEvent& event)
{

}
