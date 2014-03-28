#ifndef GEIMFRAME_H
#define GEIMFRAME_H

//(*Headers(GEIMFrame)
//*)
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/statusbr.h>

#include "ImagePanel.h"

class GEIMFrame: public wxFrame
{
    public:

        GEIMFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~GEIMFrame();

    private:
        void InitFrame();

        //(*Handlers(GEIMFrame)
        //*)
        void OnClose(wxCloseEvent& event);
        // menu or tool-button command
        void OnFileOpen(wxCommandEvent& event);
        void OnFileClose(wxCommandEvent& event);
        void OnFileCloseUpdate(wxUpdateUIEvent& event);
        void OnDt(wxCommandEvent& event);
        void OnDtUpdate(wxUpdateUIEvent& event);
        void OnMt(wxCommandEvent& event);
        void OnMtUpdate(wxUpdateUIEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        // image operate
        void OnZoomIN(wxCommandEvent& event);
        void OnZoomOut(wxCommandEvent& event);
        void OnZoomRect(wxCommandEvent& event);
        void OnZoomFit(wxCommandEvent& event);
        void OnZoomActual(wxCommandEvent& event);
        void OnImgMove(wxCommandEvent& event);
        void OnBtnsUpdate(wxUpdateUIEvent& event);

        //(*Identifiers(GEIMFrame)
        //*)
        static const long ID_PANEL_MAIN;
        static const long ID_STATUSBAR_MAIN;
        static const long ID_CMD_DT;
        static const long ID_CMD_MT;
        // tools buttons
        static const long ID_BMPBTN_IMG_ZOOMIN;
        static const long ID_BMPBTN_IMG_ZOOMOUT;
        static const long ID_BMPBTN_IMG_ZOOMRECT;
        static const long ID_BMPBTN_IMG_ZOOMFIT;
        static const long ID_BMPBTN_IMG_ZOOMACTUAL;
        static const long ID_BMPBTN_IMG_MOVE;

        //(*Declarations(GEIMFrame)
        //*)
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

        /**< array contain origin images' pointer */
        wxArrayPtrVoid m_aryImgs;
        /**< array contain origin images' pointer for dispaly */
        wxArrayPtrVoid m_aryImgsDisp;
        /**< array contain images' pointer for detected*/
        wxArrayPtrVoid m_aryImgsDt;

       /**< panel for image display */
        wxArrayPtrVoid m_aryPanels;

        DECLARE_EVENT_TABLE()
};

#endif // GEIMFRAME_H
