#include "GEIMFrame.h"

#include <stack>

#include <wx/wx.h>
#include <wx/ffile.h>

#include "AboutDlg.h"
#include "SpotDtDlg.h"
#include "DispDlg.h"
#include "EffectPar.h"
#include "SpotDtThread.h"
#include "SpotMtThread.h"
#include "Graying.h"

const long GEIMFrame::ID_PANEL_MAIN = wxNewId();
const long GEIMFrame::ID_STATUSBAR_MAIN = wxNewId();
const long GEIMFrame::ID_CMD_DT = wxNewId();
const long GEIMFrame::ID_CMD_DT_SAVE = wxNewId();
const long GEIMFrame::ID_CMD_DT_LOAD = wxNewId();
const long GEIMFrame::ID_CMD_MT = wxNewId();
const long GEIMFrame::ID_CMD_MT_SAVE = wxNewId();
const long GEIMFrame::ID_CMD_MT_LOAD = wxNewId();
// tools buttons
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMIN = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMOUT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMRECT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMFIT = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_ZOOMACTUAL = wxNewId();
const long GEIMFrame::ID_BMPBTN_IMG_MOVE = wxNewId();
const long GEIMFrame::ID_BMPBTN_SEL_SPOT = wxNewId();
const long GEIMFrame::ID_BMPBTN_DEL_SPOT = wxNewId();
const long GEIMFrame::ID_BMPBTN_SEL_PAIR = wxNewId();
const long GEIMFrame::ID_BMPBTN_DEL_PAIR = wxNewId();


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
	//pMenuItem = new wxMenuItem(pMenu, wxID_SAVE, _("&Save\tCtrl-S"), _("Save"), wxITEM_NORMAL);
	//pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, wxID_CLOSE, _("&Close\tCtrl-W"), _("Close Project"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenu->AppendSeparator();
	pMenuItem = new wxMenuItem(pMenu, wxID_EXIT, _("&Exit\tAlt-F4"), _("Exit Program"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	m_pMenuBarMain->Append(pMenu, _("&File"));
	pMenu = new wxMenu();
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_DT, _("&Detect\tCtrl-D"), _("Detect Spots"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_DT_SAVE, _("Save Detect Result(&F)\tCtrl-F"), _("Save Detect"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_DT_LOAD, _("Load Detect Result(&G)\tCtrl-G"), _("Load Detect"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_MT, _("&Match\tCtrl-M"), _("Match Spots"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_MT_SAVE, _("Save Match Result(&N)\tCtrl-N"), _("Save Match"), wxITEM_NORMAL);
	pMenu->Append(pMenuItem);
	pMenuItem = new wxMenuItem(pMenu, ID_CMD_MT_LOAD, _("Load Match Result(&B)\tCtrl-B"), _("Load Match"), wxITEM_NORMAL);
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

				pBoxSizerTools->AddSpacer(2);

				pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_SEL_SPOT, wxBitmap(_T("./skin/SelSpot.png"), wxBITMAP_TYPE_PNG));
				pBmpBtn->SetToolTip(_("Select Spot"));
				pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

				pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_DEL_SPOT, wxBitmap(_T("./skin/DelSpot.png"), wxBITMAP_TYPE_PNG));
				pBmpBtn->SetToolTip(_("Delete Spot"));
				pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

				pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_SEL_PAIR, wxBitmap(_T("./skin/SelPair.png"), wxBITMAP_TYPE_PNG));
				pBmpBtn->SetToolTip(_("Select Pair"));
				pBoxSizerTools->Add(pBmpBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

				pBmpBtn = new wxBitmapButton(m_pPanelMain, ID_BMPBTN_DEL_PAIR, wxBitmap(_T("./skin/DelPair.png"), wxBITMAP_TYPE_PNG));
				pBmpBtn->SetToolTip(_("Delete Pair"));
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
	Connect(ID_CMD_DT_SAVE, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnDtSave);
	Connect(ID_CMD_DT_SAVE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnDtSaveUpdate);
	Connect(ID_CMD_DT_LOAD, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnDtLoad);
	Connect(ID_CMD_DT_LOAD, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnDtLoadUpdate);
	Connect(ID_CMD_MT, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnMt);
	Connect(ID_CMD_MT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnMtUpdate);
	Connect(ID_CMD_MT_SAVE, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnMtSave);
	Connect(ID_CMD_MT_SAVE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnMtSaveUpdate);
	Connect(ID_CMD_MT_LOAD, wxEVT_MENU, (wxObjectEventFunction)&GEIMFrame::OnMtLoad);
	Connect(ID_CMD_MT_LOAD, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnMtLoadUpdate);
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
	Connect(ID_BMPBTN_SEL_SPOT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnSelSpot);
	Connect(ID_BMPBTN_SEL_SPOT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_DEL_SPOT, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnDelSpot);
	Connect(ID_BMPBTN_DEL_SPOT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_SEL_PAIR, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnSelPair);
	Connect(ID_BMPBTN_SEL_PAIR, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	Connect(ID_BMPBTN_DEL_PAIR, wxEVT_BUTTON, (wxObjectEventFunction)&GEIMFrame::OnDelPair);
	Connect(ID_BMPBTN_DEL_PAIR, wxEVT_UPDATE_UI, (wxObjectEventFunction)&GEIMFrame::OnBtnsUpdate);
	// thread event
	Connect(SpotDtThread::ID, wxEVT_THREAD, (wxObjectEventFunction)&GEIMFrame::OnThreadDt);
	Connect(SpotMtThread::ID, wxEVT_THREAD, (wxObjectEventFunction)&GEIMFrame::OnThreadMt);
}

GEIMFrame::~GEIMFrame()
{
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

/**< display the detect result */
bool GEIMFrame::DispDtResult()
{
	bool  bRet = true;
	// traverse all the image
	size_t nNum = m_aryImgs.size();
	if (m_lsDtResult.size() != nNum)
		return false;
	for (size_t i = 0; i < nNum; ++i)
	{
		// get the diplay image
		wxImage* pImgDisp = static_cast<wxImage*>(m_aryImgsDisp.Item(i));
		if (pImgDisp == nullptr)
		{
			wxASSERT_MSG(pImgDisp != nullptr, _T("get display image error"));
			bRet = true;
			break;
		}
		// get detection result
		auto it = m_lsDtResult.begin();	for (size_t j = 0; j < i; ++j)	++it;
		ST_DTRESULT& stResult = *it;

		// display the result
		unsigned char* pDest = pImgDisp->GetData();
		int iW = pImgDisp->GetWidth();
		int iWb = iW*3;
		int iH = pImgDisp->GetHeight();
		int iN = iW*iH;
		PST_RGB pFlag = stResult.pData;
		// draw the spot edge
		for (int i = 0; i < iN; ++i)
		{
			if(pFlag->g > 0)	// edge
			{
				pDest[0] = 0u;
				pDest[1] = 255u;
				pDest[2] = 0u;
			}
			else if(pFlag->b > 0)	// inner
			{
				pDest[0] = 0u;
				pDest[1] = 0u;
				pDest[2] = 255u;
			}
			else	// image
			{
				pDest[0] = pFlag->r;
				pDest[1] = pFlag->r;
				pDest[2] = pFlag->r;
			}
			pDest += 3;
			++pFlag;
		}
		// draw the spot center
		int iSpotN = (int)stResult.pLs->size();
		if (iSpotN <= 0)
			continue;
		pDest = pImgDisp->GetData();
		pFlag = stResult.pData;
		for (auto it = stResult.pLs->begin(); it != stResult.pLs->end(); ++it)
		{
			ST_SPOT_NODE& spot = *it;
			// 绘制蛋白点+
			int x = spot.x;
			int y = spot.y;
			unsigned char* pPix = pDest + (y*iW + x)*3;
			// -
			for (int i = -3; i <= 3; ++i)
			{
				if (x+i < 0 || x+i >= iW)
					continue;
				unsigned char* pT = pPix + i*3;
				pT[0] = 255;
				pT[1] = 0;
				pT[2] = 0;
			}
			// |
			for (int i = -3; i <= 3; ++i)
			{
				if (y+i < 0 || y+i >= iH)
					continue;
				unsigned char* pT = pPix + i*iWb;
				pT[0] = 255;
				pT[1] = 0;
				pT[2] = 0;
			}
		}
	}
	return bRet;
}

/**< add a spot */
bool GEIMFrame::AddASpot(int idImg, const wxRect& rect)
{
	bool bRet = true;

	// spot index
	int id_spot = -1;

	// get the specified image detect result
	if (idImg < 0 || idImg >= (int)m_aryImgs.size())
		return false;
    if (m_lsDtResult.size() != m_aryImgs.size())
	{
		// release pre result
		SpotDtThread::DestroyDtResult(&m_lsDtResult);

		for (size_t i = 0; i < m_aryImgs.size(); ++i)
		{
			wxImage *pimg = (wxImage*)m_aryImgs.Item(i);
			ST_RGB *pdata = (ST_RGB *)pimg->GetData();
			int isize = pimg->GetWidth()*pimg->GetHeight();
			ST_DTRESULT stResult;
			stResult.pData = (ST_RGB *)new char[isize*3];
			memset(stResult.pData, 0, isize*3);
			for (int n = 0; n < isize; ++n)
				stResult.pData[n].r = pdata[n].g;
			stResult.pLs = new LS_SPOTS;
			m_lsDtResult.push_back(stResult);
		}
		id_spot = 0;
	}
    wxImage& imgSrc = *(wxImage*)m_aryImgs.Item(idImg);
    auto it = m_lsDtResult.begin();
    for (int i = 0; i < idImg; ++i) ++it;
    ST_DTRESULT& stResult = *it;
    if (id_spot < 0)
		id_spot = stResult.pLs->back().id + 1;

    // find if the rect already thers are some spots
    // do later

    // draw ellipse in temp image, used for statistic the spot infomation
    wxBitmap bmp_mem(rect.width, rect.height);
    wxMemoryDC dc_mem(bmp_mem);
    dc_mem.SetBackground(*wxBLACK_BRUSH);
    dc_mem.SetPen(*wxGREEN_PEN);	// edge is gree
    dc_mem.SetBrush(*wxBLUE_BRUSH);	// inner is blue
    dc_mem.Clear();
    dc_mem.DrawEllipse(0, 0, rect.width-1, rect.height-1);
    dc_mem.SelectObject(wxNullBitmap);
    wxImage img_spot = bmp_mem.ConvertToImage();
    // statistic spot's pixs(edge, inner)
    std::stack<std::pair<int,int>> stk_edge;
    std::stack<std::pair<int,int>> stk_inner;
    std::stack<std::pair<int,int>> stk_itra;
    int x = -1, y = -1, img_w = -1, img_h = -1;
	std::pair<int, int> coord;
	ST_RGB *ppix = nullptr, *img_data = (ST_RGB *)img_spot.GetData();
	auto func_indentify = [&stk_edge, &stk_inner, &stk_itra, &coord, &ppix]{
		if (ppix->b == 0x0ffu)	// is the inner
		{
			stk_inner.push(coord);
			stk_itra.push(coord);
			ppix->b = 0x0fu;
		}
		else if (ppix->g == 0x0ffu)
		{
			stk_edge.push(coord);
			ppix->g = 0x0fu;
		}
	};
    img_w = img_spot.GetWidth();
    img_h = img_spot.GetHeight();
    x = rect.width/2;
    y = rect.height/2;
	ppix = img_data + y*img_w + x;
	coord = std::make_pair(x, y);
	func_indentify();
	while (!stk_itra.empty())
	{
		coord = stk_itra.top();
		stk_itra.pop();
		x = coord.first;
		y = coord.second;

		if (x-1 >= 0)	// left
		{
			ppix = img_data + y*img_w + (x-1);
			coord = std::make_pair(x-1, y);
			func_indentify();
		}
		if (y-1 >= 0)	// top
		{
			ppix = img_data + (y-1)*img_w + x;
			coord = std::make_pair(x, y-1);
			func_indentify();
		}
		if (x+1 < img_w)	// right
		{
			ppix = img_data + y*img_w + (x+1);
			coord = std::make_pair(x+1, y);
			func_indentify();
		}
		if (y+1 < img_h)	// bottom
		{
			ppix = img_data + (y+1)*img_w + x;
			coord = std::make_pair(x, y+1);
			func_indentify();
		}
	}
	// add spot in detect-result-image and spots-list
	if (stk_inner.empty() || stk_edge.empty())
		bRet = false;
	else
	{
		ST_SPOT_NODE spot;
		spot.id = id_spot;
		spot.x = rect.x + rect.width/2;
		spot.y = rect.y + rect.height/2;
		spot.xm = spot.x;
		spot.ym = spot.y;
		spot.rx = rect.width/2;
		spot.ry = rect.height/2;
		spot.perimeter = (int)stk_edge.size();
		spot.area = (int)stk_inner.size();
		spot.edge = new int[spot.perimeter];
		spot.volume = 0;
		spot.mean = 0.0f;
		spot.meanBK = 0.0f;
		spot.meanIN = 0.0f;

		img_data = stResult.pData;
		img_w = imgSrc.GetWidth();
		img_h = imgSrc.GetHeight();
		int counter = 0;
		int id = 0;
		while (!stk_inner.empty())
		{
			coord = stk_inner.top();
			stk_inner.pop();
			x = rect.x + coord.first;
			y = rect.y + coord.second;
			id = y*img_w + x;
			ppix = img_data + id;

			ppix->g = 0x00u;
			ppix->b = 0x0ffu;

			spot.meanBK += ppix->r;
		}
		while (!stk_edge.empty())
		{
			coord = stk_edge.top();
			stk_edge.pop();
			x = rect.x + coord.first;
			y = rect.y + coord.second;
			id = y*img_w + x;
			ppix = img_data + id;

			ppix->g = 0x0ffu;
			ppix->b = 0x00u;

			spot.volume += ppix->r;
			spot.edge[counter++] = id;
		}
		spot.mean = spot.volume/(float)spot.area;
		spot.meanBK /= spot.perimeter;
		spot.meanIN = spot.meanIN;
		stResult.pLs->push_back(spot);
	}

    img_spot.Destroy();
    return bRet;
}

/**< delete spots in the rectangle of specified image */
bool GEIMFrame::DelSopts(int idImg, const wxRect& rect)
{
	if (idImg < 0 || idImg >= (int)m_lsDtResult.size())
		return false;
	wxImage& imgSrc = *(wxImage*)m_aryImgs.Item(idImg);
	int img_w = imgSrc.GetWidth(), img_h = imgSrc.GetHeight();
    auto it_img = m_lsDtResult.begin();
    for (int i = 0; i < idImg; ++i) ++it_img;
    ST_DTRESULT& stResult = *it_img;

    // traversing all spot, delete those locate in the rectangle
    for (auto it_spot = stResult.pLs->begin(); it_spot != stResult.pLs->end();)
	{
		ST_SPOT_NODE& spot = *it_spot;
		if (!rect.Contains(spot.x, spot.y))
			++it_spot;
		else
		{
			// remove from detect-result-image
			ST_RGB *ppix = nullptr, *img_mask = stResult.pData;
			int x = -1, y = -1, id = -1;
			std::pair<int, int> coord;
			std::stack<std::pair<int,int>> stk_itra;
			// remove edge
			for (int i = 0; i < spot.perimeter; ++i)
			{
				id = spot.edge[i];
				ppix = img_mask + id;
				ppix->g = 0x00u;
			}
			// remove inner
			auto func_inner = [&stk_itra, &coord, &ppix]{
				if (ppix->b > 0)
				{
					stk_itra.push(coord);
					ppix->b = 0x00u;
				}
			};
			x = spot.x;
			y = spot.y;
			coord = std::make_pair(x, y);
			ppix = img_mask + y*img_w + x;
			func_inner();
			while (!stk_itra.empty())
			{
				coord = stk_itra.top();
				stk_itra.pop();
				x = coord.first;
				y = coord.second;

				if (x-1 >= 0)	// left
				{
					coord = std::make_pair(x-1, y);
					ppix = img_mask + y*img_w + (x-1);
					func_inner();
				}
				if (y-1 >= 0)	// top
				{
					coord = std::make_pair(x, y-1);
					ppix = img_mask + (y-1)*img_w + x;
					func_inner();
				}
				if (x+1 < img_w)	// right
				{
					coord = std::make_pair(x+1,y);
					ppix = img_mask + y*img_w + (x+1);
					func_inner();
				}
				if (y+1 < img_h)	// bootom
				{
					coord = std::make_pair(x, y+1);
					ppix = img_mask + (y+1)*img_w + x;
					func_inner();
				}
			}
			// remove from spots-list
			delete spot.edge;
			it_spot = stResult.pLs->erase(it_spot);
		}
	}

	return true;
}

/**< save detection result for each image, file named by <imagename.ext>.dt */
bool GEIMFrame::SaveDt()
{
	if (m_aryImgs.size() != m_lsDtResult.size())
		return false;

	// traversing all image and its detection-result
	int img_id = 0;
	for (auto it_dt = m_lsDtResult.begin(); it_dt != m_lsDtResult.end(); ++it_dt)
	{
		wxImage *img_wx = (wxImage *)m_aryImgs.Item(img_id);
		ST_DTRESULT& img_dt = *it_dt;

		// get image file-name, format <imagename.ext>.dt, then create data file
		wxString img_pathname = img_wx->GetOption(wxIMAGE_OPTION_FILENAME);
		int img_w = img_wx->GetWidth();
		int img_h = img_wx->GetHeight();
		img_pathname.Append(_T(".dt"));
		wxFFile img_file;
		if (!img_file.Open(img_pathname, _T("wb")))
			continue;
		img_file.Seek(0);

		// write data
		unsigned int flag_head = 0x64746864; // "dthd"
		unsigned int flag_splite = 0x01020304u; // ""
		unsigned int flag_tail = 0x6474746cu; // "dttl"
		unsigned int version = 0x01000100u; // "1.00.01.00"
		int len_spotls = -1;
		// head
		img_file.Write(&flag_head, 4);
		// version
		img_file.Write(&version, 4);
		// image width and height
		img_file.Write(&img_w, 4);
		img_file.Write(&img_h, 4);
		// image flags
		img_file.Write(img_dt.pData, img_w*img_h*3);
		// spots count
		len_spotls = (int)img_dt.pLs->size();
		img_file.Write(&len_spotls, 4);
		// each spot
		for (auto it_spot = img_dt.pLs->begin(); it_spot != img_dt.pLs->end(); ++it_spot)
		{
			ST_SPOT_NODE& spot = *it_spot;

			img_file.Write(&spot.id, 4);
			img_file.Write(&spot.x, 4);
			img_file.Write(&spot.y, 4);
			img_file.Write(&spot.xm, 4);
			img_file.Write(&spot.ym, 4);
			img_file.Write(&spot.area, 4);
			img_file.Write(&spot.volume, 4);
			img_file.Write(&spot.perimeter, 4);
			img_file.Write(spot.edge, 4*spot.perimeter);
			img_file.Write(&spot.rx, 4);
			img_file.Write(&spot.ry, 4);
			img_file.Write(&spot.mean, 4);
			img_file.Write(&spot.meanBK, 4);
			img_file.Write(&spot.meanIN, 4);
			img_file.Write(&flag_splite, 4);
		}
		// tail
		img_file.Write(&flag_tail, 4);

		img_file.Close();

		// next image
		++img_id;
	}
	return true;
}

 /**< load detecttion result */
bool GEIMFrame::LoadDt()
{
	bool bRet = true;
	// release pre result
	SpotDtThread::DestroyDtResult(&m_lsDtResult);

	// traversing all image and read its detection-result
	int img_count = (int)m_aryImgs.size();
	for (int img_id = 0; img_id < img_count; ++img_id)
	{
		wxImage *img_wx = (wxImage *)m_aryImgs.Item(img_id);
		int img_w = img_wx->GetWidth();
		int img_h = img_wx->GetHeight();
		int img_n = img_w*img_h*3;
		ST_DTRESULT img_dt;

		unsigned int flag_head = 0x64746864u; // "dthd"
		unsigned int flag_splite = 0x01020304u; // ""
		unsigned int flag_tail = 0x6474746cu; // "dttl"
		unsigned int version = 0x01000100u;
		int len_spotls = -1;
		int temp_i = 0;
		unsigned int temp_u = 0;

		// get image file-name, format <imagename.ext>.dt, then open data file
		wxFFile img_file;
		wxString img_pathname = img_wx->GetOption(wxIMAGE_OPTION_FILENAME);
		img_pathname.Append(_T(".dt"));
		if (!wxFileName::FileExists(img_pathname))
		{
			bRet = false;
			goto _LoadDt_for;
		}
		if (!img_file.Open(img_pathname, _T("rb")))
		{
			bRet = false;
			goto _LoadDt_for;
		}
		img_file.Seek(0);

		// write data
		// head
		img_file.Read(&temp_u, 4);
		if (temp_u != flag_head)
		{
			bRet = false;
			goto _LoadDt_for;
		}
		// version mm.ss.cc
		img_file.Read(&temp_u, 4);
		if (temp_u != version)
		{
			bRet = false;
			goto _LoadDt_for;
		}
		// image width and height
		img_file.Read(&temp_i, 4);
		if (temp_i != img_w)
		{
			bRet = false;
			goto _LoadDt_for;
		}
		img_file.Read(&temp_i, 4);
		if (temp_i != img_h)
		{
			bRet = false;
			goto _LoadDt_for;
		}
		// image flags
		img_dt.pData = (ST_RGB *)new char[img_n];
		temp_u = img_file.Read(img_dt.pData, img_n);
		wxASSERT_MSG((int)temp_u == img_n, _T("Read detection-data error."));
		// spots count
		img_file.Read(&len_spotls, 4);
		// each spot
		img_dt.pLs = new LS_SPOTS;
		for (int spot_id = 0; spot_id < len_spotls; ++spot_id)
		{
			ST_SPOT_NODE spot;

			img_file.Read(&spot.id, 4);
			img_file.Read(&spot.x, 4);
			img_file.Read(&spot.y, 4);
			img_file.Read(&spot.xm, 4);
			img_file.Read(&spot.ym, 4);
			img_file.Read(&spot.area, 4);
			img_file.Read(&spot.volume, 4);
			img_file.Read(&spot.perimeter, 4);
			spot.edge = new int[spot.perimeter];
			img_file.Read(spot.edge, 4*spot.perimeter);
			img_file.Read(&spot.rx, 4);
			img_file.Read(&spot.ry, 4);
			img_file.Read(&spot.mean, 4);
			img_file.Read(&spot.meanBK, 4);
			img_file.Read(&spot.meanIN, 4);
			img_file.Read(&temp_u, 4);
			if (temp_u != flag_splite)
			{
				delete spot.edge;
				bRet = false;
				goto _LoadDt_for;
			}

			img_dt.pLs->push_back(spot);
		}
		// tail
		img_file.Read(&temp_u, 4);
		if (temp_u != flag_tail)
			bRet = false;

_LoadDt_for:
		if (img_file.IsOpened())
			img_file.Close();
		if (bRet)
			m_lsDtResult.push_back(img_dt);
		else
		{
			delete img_dt.pData;
			delete img_dt.pLs;
			goto _LoadDt;
		}
	}

_LoadDt:
	if (!bRet)
		SpotDtThread::DestroyDtResult(&m_lsDtResult);
	return bRet;
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
		iRet = wxMessageBox(_("A group of images are already opened,\n SURE to open a new group of iamges?"),
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
		// new image
		wxImage* pImg = new wxImage(path);
		pImg->SetOption(wxIMAGE_OPTION_FILENAME, path);
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
	SpotMtThread::DestroyMtResult(&m_stMtResult);
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
	SpotDtThread* pThd = new SpotDtThread(this, stDtParam, m_aryImgs, m_lsDtResult);
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

void GEIMFrame::OnDtSave(wxCommandEvent& event)
{
	if (SaveDt())
		wxMessageBox(_("Save detection results SUCCESS."), _("Information"), wxOK|wxICON_INFORMATION|wxCENTER, this);
	else
		wxMessageBox(_("Save detection results FAILED."), _("Error"), wxOK|wxICON_ERROR|wxCENTER, this);
}
void GEIMFrame::OnDtSaveUpdate(wxUpdateUIEvent& event)
{
	bool bEn = true;
	size_t nNum = m_aryImgs.Count();
	bEn = nNum > 0;
	bEn = bEn && (nNum == m_lsDtResult.size());
	bEn = bEn && (m_pBusy == nullptr);
	event.Enable(bEn);
}

void GEIMFrame::OnDtLoad(wxCommandEvent& event)
{
	if (!LoadDt())
		wxMessageBox(_("Load detection results FAILED."), _("Error"), wxOK|wxICON_ERROR|wxCENTER, this);
	else
	{
		DispDtResult();
		RefreshImgs();
	}
}
void GEIMFrame::OnDtLoadUpdate(wxUpdateUIEvent& event)
{
	bool bEn = true;
	bEn = bEn && (m_aryImgs.Count() > 0);
	bEn = bEn && (m_pBusy == nullptr);
	event.Enable(bEn);
}

/**< match spots */
void GEIMFrame::OnMt(wxCommandEvent& event)
{
	SpotMtThread* pThd = new SpotMtThread(this, m_lsDtResult, m_aryImgs, m_aryImgsDisp, m_stMtResult);
	// match process
	if (pThd->Run() != wxTHREAD_NO_ERROR)	// pThd delete itself auto
	{
		wxMessageBox(_("Create match thread failed."), _("Information"), wxOK|wxICON_INFORMATION|wxCENTER, this);
		return;
	}
	// busy indicator
	m_pBusy = new wxBusyInfo(_("Please wait, matching..."), this);
}
void GEIMFrame::OnMtUpdate(wxUpdateUIEvent& event)
{
	bool bEn = true;
	size_t nNum = m_aryImgs.Count();
	bEn = nNum > 0;
	bEn = bEn && (nNum == m_lsDtResult.size());
	bEn = bEn && (m_pBusy == nullptr);
	bEn = bEn && (m_lsDtResult.size() >= 2);
	event.Enable(bEn);
}

void GEIMFrame::OnMtSave(wxCommandEvent& event)
{

}
void GEIMFrame::OnMtSaveUpdate(wxUpdateUIEvent& event)
{

}

void GEIMFrame::OnMtLoad(wxCommandEvent& event)
{

}
void GEIMFrame::OnMtLoadUpdate(wxUpdateUIEvent& event)
{

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
	if (m_lMouseFunc != ID_BMPBTN_IMG_ZOOMRECT)
		m_lMouseFunc = ID_BMPBTN_IMG_ZOOMRECT;
	else
		m_lMouseFunc = 0;
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
	if (m_lMouseFunc != ID_BMPBTN_IMG_ZOOMRECT)
		m_lMouseFunc = ID_BMPBTN_IMG_ZOOMRECT;
	else
		m_lMouseFunc = 0;
}

void GEIMFrame::OnSelSpot(wxCommandEvent& event)
{
	if (m_lMouseFunc != ID_BMPBTN_SEL_SPOT)
	{
		m_lMouseFunc = ID_BMPBTN_SEL_SPOT;

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::SEL_ELLIPSE);
		}
	}
	else
	{
		m_lMouseFunc = 0;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::NONE);
		}
	}
}

void GEIMFrame::OnDelSpot(wxCommandEvent& event)
{
	if (m_lMouseFunc != ID_BMPBTN_DEL_SPOT)
	{
		m_lMouseFunc = ID_BMPBTN_DEL_SPOT;

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::SEL_RECTANGLE);
		}
	}
	else
	{
		m_lMouseFunc = 0;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::NONE);
		}
	}
}

void GEIMFrame::OnSelPair(wxCommandEvent& event)
{
	if (m_lMouseFunc != ID_BMPBTN_SEL_PAIR)
	{
		m_lMouseFunc = ID_BMPBTN_SEL_PAIR;

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::SEL_ELLIPSE);
		}
	}
	else
	{
		m_lMouseFunc = 0;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::NONE);
		}
	}
}

void GEIMFrame::OnDelPair(wxCommandEvent&event)
{
	if (m_lMouseFunc != ID_BMPBTN_DEL_PAIR)
	{
		m_lMouseFunc = ID_BMPBTN_DEL_PAIR;

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::SEL_RECTANGLE);
		}
	}
	else
	{
		m_lMouseFunc = 0;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			pPanel->SelTools(IMGPL_CMD::NONE);
		}
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
	else if (id == ID_BMPBTN_SEL_SPOT)
	{
		pImgPanel->UpdateUI(IMGPL_CMD::SEL_ELLIPSE, event);
		event.Check(event.GetChecked() && (m_lMouseFunc == ID_BMPBTN_SEL_SPOT));
	}
	else if (id == ID_BMPBTN_DEL_SPOT)
	{
		pImgPanel->UpdateUI(IMGPL_CMD::SEL_RECTANGLE, event);
		event.Check(event.GetChecked() && (m_lMouseFunc == ID_BMPBTN_DEL_SPOT));
	}
	else if (id == ID_BMPBTN_SEL_PAIR)
	{
		pImgPanel->UpdateUI(IMGPL_CMD::SEL_ELLIPSE, event);
		event.Check(event.GetChecked() && (m_lMouseFunc == ID_BMPBTN_SEL_PAIR));
	}
	else if (id == ID_BMPBTN_DEL_PAIR)
	{
		pImgPanel->UpdateUI(IMGPL_CMD::SEL_RECTANGLE, event);
		event.Check(event.GetChecked() && (m_lMouseFunc == ID_BMPBTN_DEL_PAIR));
	}
}

/**< invoke when these iamge panels notify */
void GEIMFrame::OnImgplNtfy(wxImgplEvent& event)
{
	int id = event.GetId();
	IMGPL_CMD cmd = event.GetCMD();
	void* pParam = event.GetParam();

	// execu notify
	switch(cmd)
	{
	case IMGPL_CMD::IMG_ZRECT:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get IMG_ZRECT event param failed."));

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			// pass over itself
			if (pPanel->GetId() == id)
				continue;
			pPanel->ImgZoomRect(*prcSel);
		}
	}
	break;
	case IMGPL_CMD::IMG_MOVE:
	{
		wxSize* pszMv = static_cast<wxSize*>(pParam);
		wxASSERT_MSG(pszMv != nullptr, _T("EVT_IMGPL get IMG_MOVE event param failed."));

		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			// pass over itself
			if (pPanel->GetId() == id)
				continue;
			pPanel->ImgMove(*pszMv);
		}
	}
	break;
	case IMGPL_CMD::SEL_CIRCLE:
		break;
	case IMGPL_CMD::SEL_ELLIPSE:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get SEL_ELLIPSE event param failed."));
		if (prcSel == nullptr)
			break;

		// get selection in which image
		int iSel = -1;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			if (pPanel->GetId() == id)
			{
				iSel = (int)i;
				break;
			}
		}
		if (iSel == -1)
			break;

		if (m_lMouseFunc == ID_BMPBTN_SEL_SPOT)
		{
			AddASpot(iSel, *prcSel);
			DispDtResult();
			RefreshImgs();
		}
		else if (m_lMouseFunc == ID_BMPBTN_SEL_PAIR)
		{
			;
		}
	}
	break;
	case IMGPL_CMD::SEL_SQUARE:
		break;
	case IMGPL_CMD::SEL_RECTANGLE:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get SEL_ELLIPSE event param failed."));
		if (prcSel == nullptr)
			break;

		// get selection in which image
		int iSel = -1;
		size_t nNum = m_aryPanels.Count();
		for (size_t i = 0; i < nNum; ++i)
		{
			ImagePanel* pPanel = static_cast<ImagePanel*>(m_aryPanels.Item(i));
			if (pPanel->GetId() == id)
			{
				iSel = (int)i;
				break;
			}
		}
		if (iSel == -1)
			break;

		if (m_lMouseFunc == ID_BMPBTN_DEL_SPOT)
		{
			DelSopts(iSel, *prcSel);
			DispDtResult();
			RefreshImgs();
		}
		else if (m_lMouseFunc == ID_BMPBTN_DEL_PAIR)
		{
			;
		}
	}
	break;
	default:
		break;
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
	{
		// alter dispaly image
		DispDtResult();
		// update ui
		RefreshImgs();
	}
}

/**< finish match event*/
void GEIMFrame::OnThreadMt(wxThreadEvent& event)
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
		wxMessageBox(_("Match failed!"), _("Information"), wxOK|wxICON_INFORMATION|wxCENTER, this);
	else
	{
		// update ui
		RefreshImgs();
		// display union image
		if (m_stMtResult.pImgUnion != nullptr)
		{
			wxImage *pImg = (wxImage *)m_stMtResult.pImgUnion;
			if (pImg->IsOk())
			{
				DispDlg disp(this, wxNewId(), _("Match Result"));
				disp.SetImage(pImg);
				disp.ShowModal();
			}
		}
	}
}
