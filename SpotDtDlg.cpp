#include "SpotDtDlg.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "ImagePanel.h"


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


/**< simpl construct */
SpotDtDlg::SpotDtDlg()
{
	// initialize member
	Init();
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
	// centre dialog on the parente
	Centre();

	return bRet;
}

/**< Initialize member */
bool SpotDtDlg::Init()
{
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

					pTools->AddSpacer(2);

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
			wxStaticBoxSizer* pStep = new wxStaticBoxSizer(wxVERTICAL, this, _("Step 1: Set Parem"));
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

					pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_DT_FAINT, wxBitmap(_T("./skin/SelMin.png"), wxBITMAP_TYPE_PNG));
					pBmpBtn->SetToolTip(_("Select Minimum Spot in Image"));
					pSels->Add(pBmpBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pText = new wxStaticText(this, wxID_ANY, _("Select Minimum Spot"));
					pSels->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);

					pBmpBtn = new wxBitmapButton(this, ID_BMPBTN_DT_FAINT, wxBitmap(_T("./skin/SelMax.png"), wxBITMAP_TYPE_PNG));
					pBmpBtn->SetToolTip(_("Select Maximum Spot in Image"));
					pSels->Add(pBmpBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					pText = new wxStaticText(this, wxID_ANY, _("Select Maximum Spot"));
					pSels->Add(pText, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
				}
				pStep->Add(pSels, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0);

				wxCheckBox* pCB = new wxCheckBox(this, ID_CB_MEDIAN, _("Median Filter"));
				pStep->Add(pCB, 0, wxLEFT|wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);

				wxArrayString strs;
				strs.Add(_("3X3"));
				strs.Add(_("5X5"));
				strs.Add(_("7X7"));
				wxRadioBox* pTmpl = new wxRadioBox(this, ID_RBX_MEDIAN, _("Size"), wxDefaultPosition, wxDefaultSize,
				                                   strs, 3, wxRA_SPECIFY_COLS);
				pStep->Add(pTmpl, 0, wxLEFT|wxBOTTOM|wxALIGN_LEFT|wxALIGN_TOP, 2);

				pCB = new wxCheckBox(this, ID_CB_MEDIAN, _("Gauss Filter"));
				pStep->Add(pCB, 0, wxLEFT|wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);

				pTmpl = new wxRadioBox(this, ID_RBX_GAUSS, _("Size"), wxDefaultPosition, wxDefaultSize,
				                       strs, 3, wxRA_SPECIFY_COLS);
				pStep->Add(pTmpl, 0, wxLEFT|wxBOTTOM|wxALIGN_LEFT|wxALIGN_TOP, 2);
			}
			pRight->Add(pStep, 0, wxTOP|wxRIGHT|wxALIGN_TOP|wxEXPAND, 5);

			pStep = new wxStaticBoxSizer(wxVERTICAL, this, _("Step 2: Modify Parem"));
			{
				wxFlexGridSizer* pParLay = new wxFlexGridSizer(6, 2, 0, 0);
				{
					wxControl* pCtrl = nullptr;
					// 1
					pCtrl = new wxStaticText(this, wxID_ANY, _("Faint Coord:"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
					wxBoxSizer* pCoord = new wxBoxSizer(wxHORIZONTAL);
					{
						pCtrl = new wxTextCtrl(this,  ID_ED_FAINT_X, _T("0"));
						pCoord->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

						pCtrl = new wxStaticText(this, wxID_ANY, _(","));
						pCoord->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);

						pCtrl = new wxTextCtrl(this,  ID_ED_FAINT_Y, _T("0"));
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
					pCtrl = new wxTextCtrl(this,  ID_ED_SPOTNUM, _T("0"));
					pParLay->Add(pCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
				}
				pStep->Add(pParLay, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 2);

				wxButton* pBtn = new wxButton(this, wxID_APPLY, _("&Test Param"));
				pStep->Add(pBtn, 0, wxALL|wxALIGN_RIGHT|wxALIGN_BOTTOM, 2);
			}
			pRight->Add(pStep, 0, wxTOP|wxRIGHT|wxALIGN_TOP|wxEXPAND, 5);

			pRight->AddStretchSpacer(1);

			wxBoxSizer* pBtnLay = new wxBoxSizer(wxHORIZONTAL);
			{
				pBtnLay->AddStretchSpacer(1);

				wxButton* pBtn = new wxButton(this, wxID_OK, _("&Batch"));
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
