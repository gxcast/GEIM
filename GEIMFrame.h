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
        void OnDt(wxCommandEvent& event);
        void OnMt(wxCommandEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        // image operate
        void OnZoomIN(wxCommandEvent& event);
        void OnZoomOut(wxCommandEvent& event);
        void OnZoomRect(wxCommandEvent& event);
        void OnZoomFit(wxCommandEvent& event);
        void OnZoomActual(wxCommandEvent& event);
        void OnImgMove(wxCommandEvent& event);

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
        wxMenuBar* m_pMenuBarMain;
        /**< the status bar */
        wxStatusBar* m_pStatusBarMain;
        /**< the main panel for contain other control */
        wxPanel* m_pPanelMain;
        /**< the main szier for locate other control */
        wxBoxSizer* m_pBoxSizerMain;

        /**< current image */
        wxImage m_imgA;
       /**< panel for image display */
        ImagePanel* m_pImgPanel = nullptr;

        DECLARE_EVENT_TABLE()
};

#endif // GEIMFRAME_H
