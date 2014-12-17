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
         */
        bool DispDtResult();

        void OnClose(wxCloseEvent& event);
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


        static const long ID_PANEL_MAIN;
        static const long ID_STATUSBAR_MAIN;
        static const long ID_CMD_DT;
        static const long ID_CMD_DT_SAVE;
        static const long ID_CMD_DT_LOAD;
        static const long ID_CMD_MT;
        static const long ID_CMD_MT_SAVE;
        static const long ID_CMD_MT_LOAD;
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
        /**< sizer of image panel */
        wxBoxSizer* m_pBoxSizerImg = nullptr;
        /**< busy indicator */
        wxBusyInfo* m_pBusy = nullptr;

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
