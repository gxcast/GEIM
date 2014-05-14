#include "SpotDtDlg.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/busyinfo.h>
#include <math.h>

#include "ImagePanel.h"
#include "EffectPar.h"
#include "SpotDt.h"

// tools buttons
const long SpotDtDlg::ID_BMPBTN_IMG_ZOOMIN = wxNewId();
const long SpotDtDlg::ID_BMPBTN_IMG_ZOOMOUT = wxNewId();
const long SpotDtDlg::ID_BMPBTN_IMG_ZOOMRECT = wxNewId();
const long SpotDtDlg::ID_BMPBTN_IMG_ZOOMFIT = wxNewId();
const long SpotDtDlg::ID_BMPBTN_IMG_ZOOMACTUAL = wxNewId();
const long SpotDtDlg::ID_BMPBTN_IMG_MOVE = wxNewId();
// image selector
const long SpotDtDlg::ID_CI_IMAGE = wxNewId();
// detect param btn
const long SpotDtDlg::ID_BMPBTN_DT_FAINT = wxNewId();
const long SpotDtDlg::ID_BMPBTN_DT_MIN = wxNewId();
const long SpotDtDlg::ID_BMPBTN_DT_MAX = wxNewId();
const long SpotDtDlg::ID_CB_BKGCORRECT = wxNewId();
// filter param
const long SpotDtDlg::ID_CB_MEDIAN = wxNewId();
const long SpotDtDlg::ID_CB_GAUSS = wxNewId();
const long SpotDtDlg::ID_RBX_MEDIAN = wxNewId();
const long SpotDtDlg::ID_RBX_GAUSS = wxNewId();
// detect param edit
const long SpotDtDlg::ID_ED_FAINT_X = wxNewId();
const long SpotDtDlg::ID_ED_FAINT_Y = wxNewId();
const long SpotDtDlg::ID_SPD_FAINT_T = wxNewId();
const long SpotDtDlg::ID_SP_MIN = wxNewId();
const long SpotDtDlg::ID_SP_MAX = wxNewId();
const long SpotDtDlg::ID_SPD_ASPECT = wxNewId();
const long SpotDtDlg::ID_ED_SPOTNUM = wxNewId();

IMPLEMENT_CLASS(SpotDtDlg, wxDialog)

BEGIN_EVENT_TABLE(SpotDtDlg, wxDialog)
END_EVENT_TABLE()


/**< simpl construct, this func is a target construct */
SpotDtDlg::SpotDtDlg()
{
}

/**< construct and create */
SpotDtDlg::SpotDtDlg(wxWindow* parent,
                     wxWindowID id /*= wxID_ANY*/,
                     const wxString& title /*= _("Spot Detection")*/,
                     const wxPoint& pos /*= wxDefaultPosition*/,
                     const wxSize& sz /*= wxDefaultSize*/,
                     long style /*= wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER*/,
                     const wxString& name /*= wxDialogNameStr*/)
	: SpotDtDlg()	// delegate construct
{
	Create(parent, id, title, pos, sz, style, name);
}

/**< destruct */
SpotDtDlg::~SpotDtDlg()
{
}

/**< create dialog */
bool SpotDtDlg::Create(wxWindow* parent,
                       wxWindowID id /*= wxID_ANY*/,
                       const wxString& title /*= _("Spot Detection")*/,
                       const wxPoint& pos /*= wxDefaultPosition*/,
                       const wxSize& sz /*= wxDefaultSize*/,
                       long style /*= wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER*/,
                       const wxString& name /*= wxDialogNameStr*/)
{
	bool bRet = false;
	// set extra styles
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
	// create
	bRet = wxDialog::Create(parent, id, title, pos, sz, style, name);
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

/**< Initialize member, inlcude gel images */
bool SpotDtDlg::Init(const wxArrayPtrVoid* pAryImgs)
{
	// Calibration
	if (pAryImgs == NULL)
	{
		wxASSERT_MSG(false, _T("The images' array is invalid."));
		return false;
	}

	// image choice control
	wxChoice* pCI = dynamic_cast<wxChoice*>(FindWindow(ID_CI_IMAGE));
	wxASSERT_MSG(pCI != nullptr, _T("Get image choice control failed."));
	pCI->Clear();

	// create the new iamge array
	size_t nNum = pAryImgs->Count();
	wxASSERT_MSG(nNum > 0, _T("There's no image in array."));
	m_pAryImgs = pAryImgs;
	bool bFirst = true;
	for(size_t i = 0; i < nNum; ++i)
	{
		wxImage* pImg = static_cast<wxImage*>(m_pAryImgs->Item(i));
		if (pImg == nullptr)
		{
			wxASSERT_MSG(false, _T("Get a image from array failed."));
			continue;
		}

		// fill the image choice control
		if (!pImg->HasOption(wxIMAGE_OPTION_FILENAME))
		{
			wxASSERT_MSG(false, _T("The image has no option filename."));
			continue;
		}
		const wxString& nm = pImg->GetOption(wxIMAGE_OPTION_FILENAME);
		pCI->Append(nm, reinterpret_cast<void*>(i));

		// default sel the first image
		if (bFirst)
		{
			bFirst = false;
			pCI->SetSelection(0);

			// copy the first iamge to display
			if (m_imgDisp.IsOk())
				m_imgDisp.Destroy();
			m_imgDisp = pImg->Copy();
			m_pImgPanel->SetImg(m_imgDisp);
		}
	}
	wxASSERT_MSG(!bFirst, _T("There's no valid image in array."));

	return true;
}

/**< create controls and layout */
bool SpotDtDlg::CreateControl()
{
	wxBoxSizer* pTopSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxBoxSizer* pLeft = new wxBoxSizer(wxVERTICAL);
		{
			wxBoxSizer* pIT = new wxBoxSizer(wxHORIZONTAL);
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
				pIT->Add(pTools, 0, wxLEFT|wxALIGN_CENTER, 3);

				// image panel
				m_pImgPanel = new ImagePanel(this, wxID_ANY, wxDefaultPosition, wxSize(600, 500) );
				pIT->Add(m_pImgPanel, 1, wxALL|wxALIGN_CENTER|wxEXPAND,  3);
			}
			pLeft->Add(pIT, 1,  wxALL|wxALIGN_CENTER|wxEXPAND, 0);

			// Image Selection
			wxBoxSizer* pSelImg = new wxBoxSizer(wxHORIZONTAL);
			{
				wxStaticText* pText = new wxStaticText(this, wxID_ANY, _("Select Image:"));
				pSelImg->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

				wxChoice* pCiImg = new wxChoice(this, ID_CI_IMAGE);
				pSelImg->Add(pCiImg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5);

				pSelImg->AddStretchSpacer(1);
			}
			pLeft->Add(pSelImg, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
		}
		pTopSizer->Add(pLeft, 1, wxALL|wxALIGN_LEFT|wxEXPAND, 0);

		wxBoxSizer* pRight = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer* pStep = new wxStaticBoxSizer(wxVERTICAL, this, _("Step 1: Set Param"));
			{
				wxFlexGridSizer* pSels = new wxFlexGridSizer(3, 2, 0, 0);
				{
					wxBitmapButton* pBmpBtn = nullptr;
					wxStaticText* pText = nullptr;

					pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_DT_FAINT, wxBitmap(_T("./skin/SelFaint.png"), wxBITMAP_TYPE_PNG));
					pBmpBtn->SetToolTip(_("Select Faint Spot in Image"));
					pSels->Add(pBmpBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pText = new wxStaticText(this, wxID_ANY, _("Select Faint Spot"));
					pSels->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

					pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_DT_MIN, wxBitmap(_T("./skin/SelMin.png"), wxBITMAP_TYPE_PNG));
					pBmpBtn->SetToolTip(_("Select Minimum Spot in Image"));
					pSels->Add(pBmpBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pText = new wxStaticText(this, wxID_ANY, _("Select Minimum Spot"));
					pSels->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

					pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_DT_MAX, wxBitmap(_T("./skin/SelMax.png"), wxBITMAP_TYPE_PNG));
					pBmpBtn->SetToolTip(_("Select Maximum Spot in Image"));
					pSels->Add(pBmpBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pText = new wxStaticText(this, wxID_ANY, _("Select Maximum Spot"));
					pSels->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
				}
				pStep->Add(pSels, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0);

				wxCheckBox* pCB = new wxCheckBox(this, ID_CB_BKGCORRECT, _("Background Correct"));
				pStep->Add(pCB, 0, wxLEFT|wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);

				pCB = new wxCheckBox(this, ID_CB_MEDIAN, _("Median Filter"));
				pStep->Add(pCB, 0, wxLEFT|wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);

				wxArrayString strs;
				strs.Add(_("3X3"));
				strs.Add(_("5X5"));
				strs.Add(_("7X7"));
				wxRadioBox* pTmpl = new wxRadioBox(this, ID_RBX_MEDIAN, _("Size"), wxDefaultPosition, wxDefaultSize,
				                                   strs, 3, wxRA_SPECIFY_COLS);
				pStep->Add(pTmpl, 0, wxLEFT|wxBOTTOM|wxALIGN_LEFT|wxALIGN_TOP, 2);

				pCB = new wxCheckBox(this, ID_CB_GAUSS, _("Gauss Filter"));
				pStep->Add(pCB, 0, wxLEFT|wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);

				pTmpl = new wxRadioBox(this, ID_RBX_GAUSS, _("Size"), wxDefaultPosition, wxDefaultSize,
				                       strs, 3, wxRA_SPECIFY_COLS);
				pStep->Add(pTmpl, 0, wxLEFT|wxBOTTOM|wxALIGN_LEFT|wxALIGN_TOP, 2);
			}
			pRight->Add(pStep, 0, wxTOP|wxRIGHT|wxALIGN_TOP|wxEXPAND, 5);

			pStep = new wxStaticBoxSizer(wxVERTICAL, this, _("Step 2: Modify Param"));
			{
				wxFlexGridSizer* pParLay = new wxFlexGridSizer(6, 2, 0, 0);
				{
					wxControl* pCtrl = nullptr;
					// 1
					pCtrl = new wxStaticText(this, wxID_ANY, _("Faint Coord:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					wxBoxSizer* pCoord = new wxBoxSizer(wxHORIZONTAL);
					{
						pCtrl = new wxTextCtrl(this,  ID_ED_FAINT_X, _T("0"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT|wxTE_READONLY, wxTextValidator(wxFILTER_DIGITS));
						pCoord->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

						pCtrl = new wxStaticText(this, wxID_ANY, _(","));
						pCoord->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

						pCtrl = new wxTextCtrl(this,  ID_ED_FAINT_Y, _T("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxTextValidator(wxFILTER_DIGITS));
						pCoord->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
					}
					pParLay->Add(pCoord, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
					// 2
					pCtrl = new wxStaticText(this, wxID_ANY, _("Faint Thre:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pCtrl = new wxSpinCtrlDouble(this, ID_SPD_FAINT_T, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT,
					                             0.0, 100.0, 0.0, 0.5);
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
					// 3
					pCtrl = new wxStaticText(this, wxID_ANY, _("Min Rad:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pCtrl = new wxSpinCtrl(this, ID_SP_MIN);
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
					// 4
					pCtrl = new wxStaticText(this, wxID_ANY, _("Max Rad:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pCtrl = new wxSpinCtrl(this, ID_SP_MAX);
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
					// 5
					pCtrl = new wxStaticText(this, wxID_ANY, _("Min Aspect:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pCtrl = new wxSpinCtrlDouble(this, ID_SPD_ASPECT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT,
					                             0.0, 100.0, 0.0, 0.5);
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
					// 6
					pCtrl = new wxStaticText(this, wxID_ANY, _("Spot Num:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pCtrl = new wxTextCtrl(this,  ID_ED_SPOTNUM, _T("0"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT|wxTE_READONLY, wxTextValidator(wxFILTER_DIGITS));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
				}
				pStep->Add(pParLay, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 2);
			}
			pRight->Add(pStep, 0, wxTOP|wxRIGHT|wxALIGN_TOP|wxEXPAND, 5);

			pRight->AddStretchSpacer(1);

			wxBoxSizer* pBtnLay = new wxBoxSizer(wxHORIZONTAL);
			{
				pBtnLay->AddStretchSpacer(1);

				wxButton* pBtn = new wxButton(this, wxID_APPLY, _("&Test Param"));
				pBtnLay->Add(pBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 3);

				pBtn = new wxButton(this, wxID_OK, _("&Batch"));
				pBtnLay->Add(pBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 3);

				pBtn = new wxButton(this, wxID_CANCEL, _("&Exit"));
				pBtnLay->Add(pBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 3);
			}
			pRight->Add(pBtnLay, 0, wxBOTTOM|wxRIGHT|wxALIGN_BOTTOM|wxEXPAND, 2);
		}
		pTopSizer->Add(pRight, 0, wxALL|wxALIGN_RIGHT|wxEXPAND, 0);
	}
	SetSizer(pTopSizer);
	pTopSizer->Fit(this);
	pTopSizer->SetSizeHints(this);

	return true;
}

/**< dynamic event map */
bool SpotDtDlg::DyEventMap()
{
	// image operate bottons
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnZoomIN);
	Connect(ID_BMPBTN_IMG_ZOOMIN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnZoomOut);
	Connect(ID_BMPBTN_IMG_ZOOMOUT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnZoomRect);
	Connect(ID_BMPBTN_IMG_ZOOMRECT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnZoomFit);
	Connect(ID_BMPBTN_IMG_ZOOMFIT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnZoomActual);
	Connect(ID_BMPBTN_IMG_ZOOMACTUAL, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnImgMove);
	Connect(ID_BMPBTN_IMG_MOVE, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_DT_FAINT, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnDtFaint);
	Connect(ID_BMPBTN_DT_FAINT, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_DT_MIN, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnDtMin);
	Connect(ID_BMPBTN_DT_MIN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(ID_BMPBTN_DT_MAX, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnDtMax);
	Connect(ID_BMPBTN_DT_MAX, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnsUpdate);
	Connect(m_pImgPanel->GetId(), wxEVT_IMGPL, (wxObjectEventFunction)&SpotDtDlg::OnImgplNtfy);

	// other controls
	Connect(ID_RBX_MEDIAN, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnRbxMedianUpdate);
	Connect(ID_RBX_GAUSS, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnRbxGaussUpdate);
	Connect(ID_SP_MAX, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnSpMaxUpdate);
	Connect(ID_CI_IMAGE, wxEVT_CHOICE, (wxObjectEventFunction)&SpotDtDlg::OnCiImage);
	Connect(wxID_APPLY, wxEVT_BUTTON, (wxObjectEventFunction)&SpotDtDlg::OnBtnTestParam);
	Connect(wxID_APPLY, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnProcUpdate);
	Connect(wxID_OK, wxEVT_UPDATE_UI, (wxObjectEventFunction)&SpotDtDlg::OnBtnProcUpdate);

	return true;
}

/**< obtain detect param from ui control*/
bool SpotDtDlg::GetParam()
{
	// get detect param
	wxString strValue = wxEmptyString;
	long lValue =  0l;
	wxTextCtrl* pEdit = nullptr;
	wxSpinCtrl* pSp = nullptr;
	wxSpinCtrlDouble* pSpd = nullptr;
	wxCheckBox* pCkb = nullptr;
	wxRadioBox* pRdbx = nullptr;

	// faint x
	pEdit = dynamic_cast<wxTextCtrl*>(FindWindow(ID_ED_FAINT_X));
	wxASSERT_MSG(pEdit != nullptr, _T("Get Fain X control failed."));
	if (pEdit != nullptr)
	{
		strValue = pEdit->GetValue();
		strValue.ToLong(&lValue);
		m_stDtParam.ptFaint.x = (int)lValue;
	}
	// faint y
	pEdit = dynamic_cast<wxTextCtrl*>(FindWindow(ID_ED_FAINT_Y));
	wxASSERT_MSG(pEdit != nullptr, _T("Get Fain Y control failed."));
	if (pEdit != nullptr)
	{
		strValue = pEdit->GetValue();
		strValue.ToLong(&lValue);
		m_stDtParam.ptFaint.y = (int)lValue;
	}
	// faint shreshold
	pSpd = dynamic_cast<wxSpinCtrlDouble*>(FindWindow(ID_SPD_FAINT_T));
	wxASSERT_MSG(pSpd != nullptr, _T("Get Faint Shreshold control failed."));
	if (pSpd != nullptr)
		m_stDtParam.dFaint = pSpd->GetValue();
	// min spot radius
	pSp = dynamic_cast<wxSpinCtrl*>(FindWindow(ID_SP_MIN));
	wxASSERT_MSG(pSp != nullptr, _T("Get min spot radius control failed."));
	if (pSp != nullptr)
		m_stDtParam.iMinRad = pSp->GetValue();
	// max spot radius
	pCkb = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_BKGCORRECT));
	wxASSERT_MSG(pCkb != nullptr, _T("Get background correct control failed."));
	if (pCkb != nullptr && pCkb->IsChecked())
	{
		pSp = dynamic_cast<wxSpinCtrl*>(FindWindow(ID_SP_MIN));
		wxASSERT_MSG(pSp != nullptr, _T("Get min spot radius control failed."));
		if (pSp != nullptr)
			m_stDtParam.iMaxRad = pSp->GetValue();
	}
	else
		m_stDtParam.iMaxRad = -1;
	// minimum aspect
	pSpd = dynamic_cast<wxSpinCtrlDouble*>(FindWindow(ID_SPD_ASPECT));
	wxASSERT_MSG(pSpd != nullptr, _T("Get aspect spin control failed."));
	if (pSpd != nullptr)
		m_stDtParam.dAspect = pSpd->GetValue();

	// median filter
	pCkb = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_MEDIAN));
	wxASSERT_MSG(pCkb != nullptr, _T("Get median checkbox control failed."));
	if (pCkb != nullptr && pCkb->IsChecked())
	{
		pRdbx = dynamic_cast<wxRadioBox*>(FindWindow(ID_RBX_MEDIAN));
		wxASSERT_MSG(pRdbx != nullptr, _T("Get median radiobox control failed."));
		if (pRdbx != nullptr)
			m_stDtParam.iMedianFlt = pRdbx->GetSelection();
	}
	else
		m_stDtParam.iMedianFlt = -1;

	// gaussian filter
	pCkb = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_GAUSS));
	wxASSERT_MSG(pCkb != nullptr, _T("Get gaussian checkbox control failed."));
	if (pCkb != nullptr && pCkb->IsChecked())
	{
		pRdbx = dynamic_cast<wxRadioBox*>(FindWindow(ID_RBX_GAUSS));
		wxASSERT_MSG(pRdbx != nullptr, _T("Get gaussian radiobox control failed."));
		if (pRdbx != nullptr)
			m_stDtParam.iGaussFlt = pRdbx->GetSelection();
	}
	else
		m_stDtParam.iGaussFlt = -1;

	return true;
}

/**< obtain the faint spot in special circle */
bool SpotDtDlg::GetFaintSpot(const wxRect& rc)
{
	if (!m_imgDisp.IsOk() || rc.width <= 0)
		return false;

	unsigned char* pSrc = m_imgDisp.GetData();
	int iW = m_imgDisp.GetWidth();
	int iH = m_imgDisp.GetHeight();
	// the circle
	int iBx = rc.x - rc.width, iEx = rc.x + rc.width;
	int iBy = rc.y - rc.width, iEy = rc.y + rc.width;
	if (iBx < 0)
		iBx = 0;
	if (iEx >= iW)
		iEx = iW -1;
	if (iBy < 0)
		iBy = 0;
	if (iEy <= iH)
		iEy = iH -1;
	// traverse the circle's pixel, find the min-intensity spot
	wxPoint ptMin = {0, 0};
	unsigned char pixMin = 255;
	unsigned char* pLine = pSrc + iBy*iW*3;
	for (int y = iBy; y <= iEy; ++y)
	{
		unsigned char* pPix = pLine + iBx*3;
		for (int x = iBx; x <= iEx; ++x)
		{
			// if in the circle
			if((y-rc.y)*(y - rc.y) + (x-rc.x)*(x-rc.x) <= rc.width*rc.width)
			{
				if (pPix[0] < pixMin)
				{
					pixMin = pPix[0];
					ptMin.y = y;
					ptMin.x = x;
				}
			}

			pPix += 3;
		}
		pLine += iW*3;
	}

	// the faint spot, dispaly the coord
	wxString str = wxEmptyString;
	wxTextCtrl* pEdit = dynamic_cast<wxTextCtrl*>(FindWindow(ID_ED_FAINT_X));
	wxASSERT_MSG(pEdit != nullptr, _T("Get the faint spot x control failed."));
	if (pEdit != nullptr)
	{
		str.Printf(_T("%d"), ptMin.x);
		pEdit->SetValue(str);
	}
	pEdit = dynamic_cast<wxTextCtrl*>(FindWindow(ID_ED_FAINT_Y));
	wxASSERT_MSG(pEdit != nullptr, _T("Get the faint spot x control failed."));
	if (pEdit != nullptr)
	{
		str.Printf(_T("%d"), ptMin.y);
		pEdit->SetValue(str);
	}

	return true;
}

/**< zoom in image */
void SpotDtDlg::OnZoomIN(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomIn();
}

/**< zoom out image */
void SpotDtDlg::OnZoomOut(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomOut();
}

/**< zoom image depend sel rect */
void SpotDtDlg::OnZoomRect(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomRect();
}

/**< zoom image fit the wnd */
void SpotDtDlg::OnZoomFit(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomFit();
}

/**< display image in it's actual size */
void SpotDtDlg::OnZoomActual(wxCommandEvent& event)
{
	m_pImgPanel->ImgZoomActual();
}

/**< move iamge to diaplay different region */
void SpotDtDlg::OnImgMove(wxCommandEvent& event)
{
	m_pImgPanel->ImgMove();
}

/**< select faint spot */
void SpotDtDlg::OnDtFaint(wxCommandEvent& event)
{
	m_pImgPanel->SelFaint();
}

/**< select min radius spot */
void SpotDtDlg::OnDtMin(wxCommandEvent& event)
{
	m_pImgPanel->SelMin();
}

/**< select max radius spot */
void SpotDtDlg::OnDtMax(wxCommandEvent& event)
{
	m_pImgPanel->SelMax();
}

/**< update tool btns' state */
void SpotDtDlg::OnBtnsUpdate(wxUpdateUIEvent& event)
{
	if (m_pAryImgs == nullptr)
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
	else if (id == ID_BMPBTN_DT_FAINT)
		m_pImgPanel->UpdateUI(IMGPL_CMD::SEL_FAINT, event);
	else if (id == ID_BMPBTN_DT_MIN)
		m_pImgPanel->UpdateUI(IMGPL_CMD::SEL_MIN, event);
	else if (id == ID_BMPBTN_DT_MAX)
	{
		wxCheckBox* pCbx = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_BKGCORRECT));
		wxASSERT_MSG(pCbx != nullptr, _T("Get Background checkbox control failed."));
		if (pCbx != nullptr && pCbx->IsChecked())
			m_pImgPanel->UpdateUI(IMGPL_CMD::SEL_MAX, event);
		else
			event.Enable(false);
	}
}

/**< invoke when these iamge panels notify */
void SpotDtDlg::OnImgplNtfy(wxImgplEvent& event)
{
	IMGPL_CMD cmd = event.GetCMD();
	void* pParam = event.GetParam();

	// execu notify
	switch(cmd)
	{
	case IMGPL_CMD::IMG_ZRECT:
		break;
	case IMGPL_CMD::IMG_MOVE:
		break;
	case IMGPL_CMD::SEL_FAINT:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get SEL_FAINT event param failed."));
		if (prcSel != nullptr)
			GetFaintSpot(*prcSel);
	}
	break;
	case IMGPL_CMD::SEL_MIN:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get SEL_MIN event param failed."));
		wxSpinCtrl* pSp = dynamic_cast<wxSpinCtrl*>(FindWindow(ID_SP_MIN));
		wxASSERT_MSG(pSp != nullptr, _T("Get min spot radius control failed."));
		if (pSp != nullptr)
			pSp->SetValue(prcSel->width);
	}
	break;
	case IMGPL_CMD::SEL_MAX:
	{
		wxRect* prcSel = static_cast<wxRect*>(pParam);
		wxASSERT_MSG(prcSel != nullptr, _T("EVT_IMGPL get SEL_MAX event param failed."));
		wxSpinCtrl* pSp = dynamic_cast<wxSpinCtrl*>(FindWindow(ID_SP_MAX));
		wxASSERT_MSG(pSp != nullptr, _T("Get max spot radius control failed."));
		if (pSp != nullptr)
			pSp->SetValue(prcSel->width);
	}
	break;
	default:
		break;
	}
}

/**< median template size can be modify only when it is checked  */
void SpotDtDlg::OnRbxMedianUpdate(wxUpdateUIEvent& event)
{
	wxCheckBox* pCB = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_MEDIAN));
	wxRadioBox* pRbx = dynamic_cast<wxRadioBox*>(FindWindow(ID_RBX_MEDIAN));
	if (pCB != nullptr && pRbx != nullptr)
		pRbx->Enable(pCB->IsChecked());
}

/**< gauss template size can be modify only when it is checked  */
void SpotDtDlg::OnRbxGaussUpdate(wxUpdateUIEvent& event)
{
	wxCheckBox* pCB = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_GAUSS));
	wxRadioBox* pRbx = dynamic_cast<wxRadioBox*>(FindWindow(ID_RBX_GAUSS));
	if (pCB != nullptr && pRbx != nullptr)
		pRbx->Enable(pCB->IsChecked());
}

/**< max radius only needed when background correct */
void SpotDtDlg::OnSpMaxUpdate(wxUpdateUIEvent& event)
{
	wxCheckBox* pCB = dynamic_cast<wxCheckBox*>(FindWindow(ID_CB_BKGCORRECT));
	wxSpinCtrl* pSp = dynamic_cast<wxSpinCtrl*>(FindWindow(ID_SP_MAX));
	if (pCB != nullptr && pSp != nullptr)
		pSp->Enable(pCB->IsChecked());
}

/**< change image selection */
void SpotDtDlg::OnCiImage(wxCommandEvent& event)
{
	size_t nNum = m_pAryImgs->Count();
	int iSel = event.GetSelection();
	if (iSel < 0 || iSel >= (long)nNum)
	{
		wxASSERT_MSG(false, _T("Change image selection error."));
		return;
	}

	wxImage* pImg = static_cast<wxImage*>(m_pAryImgs->Item(iSel));
	if (m_imgDisp.IsOk())
		m_imgDisp.Destroy();
	m_imgDisp = pImg->Copy();
	m_pImgPanel->SetImg(m_imgDisp);
}

/**< Test Param */
void SpotDtDlg::OnBtnTestParam(wxCommandEvent& event)
{
	wxBusyInfo wait(_("Please wait, working..."), this);
	// get param from ui
	GetParam();

	// get current image and cache
	EffectPar parEft;
	unsigned char* pIn = nullptr;
	{
		size_t nNum = m_pAryImgs->Count();
		wxChoice* pCi = dynamic_cast<wxChoice*>(FindWindow(ID_CI_IMAGE));
		wxASSERT_MSG(pCi != nullptr, _T("Get Image Switch control failed."));
		if (pCi == nullptr)
			return;
		int iSel = pCi->GetSelection();
		if (iSel < 0 || iSel >= (long)nNum)
		{
			wxASSERT_MSG(false, _T("Get current image selection error."));
			return;
		}

		wxImage* pImgCur = nullptr;
		pImgCur = static_cast<wxImage*>(m_pAryImgs->Item(iSel));
		wxASSERT_MSG(pImgCur != nullptr, _T("Get current select image error."));
		if (pImgCur == nullptr)
			return;

		// init effect data
		parEft.SetImage(pImgCur);
		// copy out the input image
		pIn = parEft.GetCache();
		memcpy(pIn, pImgCur->GetData(), (size_t)parEft.PixNum()*3);
		parEft.Input(pIn, true);
	}

	// detect
	SpotDt dt;
	dt.DtMain(&m_stDtParam, &parEft);

	// copy the resualt to the disp image
	unsigned char* pDes = m_imgDisp.GetData();
	unsigned char* pOut = parEft.Output();
	memcpy(pDes, pOut, (size_t)parEft.PixNum()*3);
	parEft.Recycle(pIn);
	// update ui
	m_pImgPanel->Refresh();
}
/**< update "Test Param" and "Batch" button's state*/
void SpotDtDlg::OnBtnProcUpdate(wxUpdateUIEvent& event)
{
	if (m_pAryImgs == nullptr)
		event.Enable(false);
	else
	{
		size_t nNum = m_pAryImgs->Count();
		event.Enable(nNum > 0);
	}
}

