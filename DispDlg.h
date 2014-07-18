#ifndef DISPDLG_H
#define DISPDLG_H

#include "GEIMDef.h"
#include "ImagePanel.h"

class DispDlg : public wxDialog
{
public:
	DispDlg();

	DispDlg(wxWindow* parent,
	        wxWindowID id=wxID_ANY,
	        const wxString& title = _("Image"),
	        const wxPoint& pos=wxDefaultPosition,
	        const wxSize& sz=wxDefaultSize);

	virtual ~DispDlg();

	bool Create(wxWindow* parent,
	           wxWindowID id=wxID_ANY,
	           const wxString& title = _("Image"),
	           const wxPoint& pos=wxDefaultPosition,
	           const wxSize& sz=wxDefaultSize);

	bool SetImage(wxImage* pImg);
	bool SetImage(unsigned char *pData, int iW, int iH);
	bool ReleaseImg();

protected:

private:
	/** \brief create controls and layout in the dialog
	 *
	 * \return bool true:success false:failed
	 *
	 */
	bool CreateControl();
	/** \brief dynamic evet map
	 *
	 * \return bool true:success false:failed
	 *
	 */
	bool DyEventMap();

	// image operate buttons
	void OnZoomIN(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnZoomRect(wxCommandEvent& event);
	void OnZoomFit(wxCommandEvent& event);
	void OnZoomActual(wxCommandEvent& event);
	void OnImgMove(wxCommandEvent& event);
	void OnBtnsUpdate(wxUpdateUIEvent& event);

	/**< control id */
	// tools buttons
	static const long ID_BMPBTN_IMG_ZOOMIN;
	static const long ID_BMPBTN_IMG_ZOOMOUT;
	static const long ID_BMPBTN_IMG_ZOOMRECT;
	static const long ID_BMPBTN_IMG_ZOOMFIT;
	static const long ID_BMPBTN_IMG_ZOOMACTUAL;
	static const long ID_BMPBTN_IMG_MOVE;

	/**< image be display */
	wxImage* m_pImgDisp = nullptr;
	/**< image panel to dispaly iamge */
	ImagePanel* m_pImgPanel = nullptr;

	DECLARE_CLASS(DispDlg)
};

#endif // DISPDLG_H
