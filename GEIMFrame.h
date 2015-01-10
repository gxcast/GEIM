#ifndef GEIMFRAME_H
#define GEIMFRAME_H

// wxwidgets' controls
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
// busy indicator
#include <wx/busyinfo.h>

#include "GEIMDef.h"
#include "ImagePanel.h"

class GEIMFrame: public wxFrame
{
public:
	GEIMFrame(wxWindow* parent,wxWindowID id = -1);
	virtual ~GEIMFrame();

private:
    /** \brief show infomation on status bar
     *
     * \param info wxString&	[IN] infomation
     * \param int interval		[IN] how long time to show
     * \param int index			[IN] which area in status bar
     * \return bool
     *
     */
	bool ShowStatusInfo(const wxString& info, int interval = -1, int index = 0);
	/** \brief refresh all the dispaly image
	 *
	 * \return bool true:success false:failed
	 *
	 */
	bool RefreshImgs();
	/** \brief display the detect result
	 *
	 * \return bool true:success false:failed
	 *
	 * pre-processed image + edge + spot center + spot id(yellow)
	 */
	bool DispDtResult();
    /** \brief display match result
     *
     * \return bool
     *
     * pair id(brown)
     */
	bool DispMtResult(int id_img = -1);
	/** \brief add a spot
	 *
	 * \param idImg int             [IN] image index
	 * \param rect const wxRect&    [IN] spot rect in the image
	 * \return bool true:success false:add failed
	 *
	 */
	bool AddASpot(int idImg, const wxRect& rect);
	/** \brief delete spots in the rectangle of specified image
	 *
	 * \param idImg int				[IN] image index
	 * \param rect const wxRect&	[IN] rectangle in specified iamge
	 * \return bool true:success false:add failed
	 *
	 */
	bool DelSopts(int idImg, const wxRect& rect);
    /** \brief add a pair
     *
     * \param idImg int				[IN] image index
     * \param rect const wxRect&	[IN] rectangle in specified iamge
     * \return int >=0 selected spot id; -1 finish add; <-1 error
     *
     * two step, select a spot in one image, then select a spot in another image
     */
	int AddAPair(int idImg, const wxRect& rect);
    /** \brief delete pairs
     *
     * \param idImg int				[IN] image index
     * \param rect const wxRect&	[IN] rectangle in specified iamge
     * \return bool
     *
     * delete pairs has spot in selected spots
     */
	bool DelPairs(int idImg, const wxRect& rect);
	/** \brief save detection result for each image, file named by <imagename.ext>.dt
	 *
	 * \return bool true:success false:add failed
	 *
	 * 4:head(4 char) 4:version(3 uchar)
	 * 4(int):image width 4(int): image height w*h*3(*):image-flag data
	 * 4(int): spot count ?(struct):spot-list
	 */
	bool SaveDt();
	/** \brief load detecttion result
	 *
	 * \return bool true:success false:add failed
	 *
	 */
	bool LoadDt();
	/** \brief save match result to file
	 *
	 * \return bool
	 *
	 * 4:head(4 char) 4:version(3 uchar)
	 * 4(int): pair count ?(struct):pair-list
	 */
	bool SaveMt();
	/** \brief load match result from file
	 *
	 * \return bool
	 *
	 */
	bool LoadMt();
    /** \brief Load or save match result as text-file, used in matalab
     *
     * \return bool
     *
     */
	bool SaveMtText();
	bool LoadMtText();

	void OnClose(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);
	// menu or tool-button command
	void OnFileOpen(wxCommandEvent& event);
	void OnFileOpenUpdate(wxUpdateUIEvent& event);
	void OnFileClose(wxCommandEvent& event);
	void OnFileCloseUpdate(wxUpdateUIEvent& event);
	void OnDt(wxCommandEvent& event);
	void OnDtUpdate(wxUpdateUIEvent& event);
	void OnDtSave(wxCommandEvent& event);
	void OnDtSaveUpdate(wxUpdateUIEvent& event);
	void OnDtLoad(wxCommandEvent& event);
	void OnDtLoadUpdate(wxUpdateUIEvent& event);
	void OnMt(wxCommandEvent& event);
	void OnMtUpdate(wxUpdateUIEvent& event);
	void OnMtSave(wxCommandEvent& event);
	void OnMtSaveUpdate(wxUpdateUIEvent& event);
	void OnMtLoad(wxCommandEvent& event);
	void OnMtSaveText(wxCommandEvent& event);
	void OnMtLoadText(wxCommandEvent& event);
	void OnMtLoadUpdate(wxUpdateUIEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnQuitUpdate(wxUpdateUIEvent& event);
	void OnAbout(wxCommandEvent& event);
	// image operate
	void OnZoomIN(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnZoomRect(wxCommandEvent& event);
	void OnZoomFit(wxCommandEvent& event);
	void OnZoomActual(wxCommandEvent& event);
	void OnImgMove(wxCommandEvent& event);
	void OnSelSpot(wxCommandEvent& event);
	void OnDelSpot(wxCommandEvent& event);
	void OnSelPair(wxCommandEvent& event);
	void OnDelPair(wxCommandEvent& event);
	void OnBtnsUpdate(wxUpdateUIEvent& event);
	void OnImgplNtfy(wxImgplEvent& event);
	// thread event
	void OnThreadDt(wxThreadEvent& event);
	void OnThreadMt(wxThreadEvent& event);


	// timer id
	static const long ID_TIMER_UI;
	// ui control
	static const long ID_PANEL_MAIN;
	static const long ID_STATUSBAR_MAIN;
	static const long ID_CMD_DT;
	static const long ID_CMD_DT_SAVE;
	static const long ID_CMD_DT_LOAD;
	static const long ID_CMD_MT;
	static const long ID_CMD_MT_SAVE;
	static const long ID_CMD_MT_LOAD;
	static const long ID_CMD_MT_SAVETEXT;
	static const long ID_CMD_MT_LOADTEXT;
	// tools buttons
	static const long ID_BMPBTN_IMG_ZOOMIN;
	static const long ID_BMPBTN_IMG_ZOOMOUT;
	static const long ID_BMPBTN_IMG_ZOOMRECT;
	static const long ID_BMPBTN_IMG_ZOOMFIT;
	static const long ID_BMPBTN_IMG_ZOOMACTUAL;
	static const long ID_BMPBTN_IMG_MOVE;
	static const long ID_BMPBTN_SEL_SPOT;
	static const long ID_BMPBTN_DEL_SPOT;
	static const long ID_BMPBTN_SEL_PAIR;
	static const long ID_BMPBTN_DEL_PAIR;

	/**< mouse function, use button's id */
	long m_lMouseFunc = 0;

	/**< the menu bar */
	wxMenuBar* m_pMenuBarMain = nullptr;
	/**< the status bar */
	wxStatusBar* m_pStatusBarMain = nullptr;
	/**< the main panel for contain other control */
	wxPanel* m_pPanelMain = nullptr;
	/**< the main szier for locate other control */
	wxBoxSizer* m_pBoxSizerMain = nullptr;
	/**< sizer of image panelDefault constructor. */
	wxBoxSizer* m_pBoxSizerImg = nullptr;
	/**< busy indicator */
	wxBusyInfo* m_pBusy = nullptr;
	/**< timer */
	wxTimer* m_pTimer = nullptr;

	/**< array contain origin images' pointer */
	wxArrayPtrVoid m_aryImgs;
	/**< array contain origin images' pointer for dispaly */
	wxArrayPtrVoid m_aryImgsDisp;

	/**< panel for image display */
	wxArrayPtrVoid m_aryPanels;

	/**< all the image's detection result */
	LS_DTRESULT m_lsDtResult;
	/**< match result */
	ST_MTRESULT m_stMtResult;

	DECLARE_EVENT_TABLE()
};

#endif // GEIMFRAME_H
