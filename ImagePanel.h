﻿#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

//(*Headers(ImagePanel)
//*)
#include <wx/panel.h>
#include <wx/geometry.h>
#include <wx/dcmemory.h>

/** \brief ImagePanel
 *
 * a panel for display image
 * and offer some zoom function to modify the disp-style
 *
 * \file ImagePanel.h
 * \author author:zhd
 * \date date:2014.03.19
 *
 */
class ImagePanel: public wxPanel
{
public:
	// construct
	ImagePanel();
	ImagePanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& sz=wxDefaultSize);
	virtual ~ImagePanel();

    /** \brief the panel whether contain a ok image
     *
     * \return bool
     *
     */
	bool IsOK() { return m_img.IsOk(); }

	/** \brief set image to dispaly
	 *
	 * \param img wxImage&	[IN] seted image
	 * \return bool			true:success false:failed
	 *
	 */
	bool SetImg(wxImage& img);
    /** \brief zoomin image
     *
     * \return bool true:success false:failed
     *
     */

	bool ImgZoomIn();
    /** \brief zoomout image
     *
     * \return bool true:success false:failed
     *
     */
	bool ImgZoomOut();
    /** \brief mouse fuc is sel the display rect
     *
     * \see bool ImgZoomRect(wxRect rcSel)
     * \return bool true:success false:failed
     *
     */
	bool ImgZoomRect();
    /** \brief dispaly image in real size
     *
     * \return bool true:success false:failed
     *
     */
	bool ImgZoomActual();
    /** \brief display image full fit wnd
     *
     * \return bool true:success false:failed
     *
     */
	bool ImgZoomFit();
    /** \brief mouse fuc is move image
     *
     * \see bool ImgMove(wxSize szMove)
     * \return bool true:success false:failed
     *
     */
	bool ImgMove();

protected:

private:
    /** \brief display image of sel rect full fit wnd
     *
     * \param wxRect rc	[IN] rect baseed of wnd
     * \return bool true:success false:failed
     *
     */
	bool ImgZoomRect(wxRect rcSel);
	/** \brief according to the specified zoom percentage
	 *
	 * \param wxDouble dScale              [IN] the specified zoom percentage
	 * \param wxPoint* pPt = nullptr    [IN] the fixed point in wnd coordinate, if nullptr, use the wnd's center
	 * \return bool true:success false:failed
	 *
	 */
	bool ImgZoomScale(wxDouble dScale, wxPoint* pPt = nullptr);
    /** \brief move image
     *
     * \param wxSize szMove	[IN] the size to move in wnd coordinate
     * \return bool true:success false:failed
     *
     */
	bool ImgMove(wxSize szMove);
    /** \brief calculate the rect of the mouse sel
     *
     * \return bool
     *
     */
	bool RegulaSelRect();

	//(*Handlers(ImagePanel)
	//*)
    /** \brief paint wnd
     *
     * \param event wxPaintEvent&	[INOUT] event
     * \return void
     *
     */
	void OnPaint(wxPaintEvent& event);
    /** \brief erase backgroud
     *
     * \param event wxEraseEvent&
     * \return void
     *
     */
	void OnErase(wxEraseEvent& event);
    /** \brief on wnd resized, do nothing to stop auto erase
     *
     * \param event wxSizeEvent&
     * \return void
     *
     */
	void OnSize(wxSizeEvent& event);
    /** \brief invoke when mouse left button down
     *
     * \param event wxMouseEvent&   [INOUT] the event
     * \return void
     *
     */
	void OnMouseLD(wxMouseEvent& event);
    /** \brief invoke when mouse left button up
     *
     * \param event wxMouseEvent&   [INOUT] the event
     * \return void
     *
     */
	void OnMouseLU(wxMouseEvent& event);
    /** \brief invoke when mouse move
     *
     * \param event wxMouseEvent&   [INOUT] the event
     * \return void
     *
     */
	void OnMouseMove(wxMouseEvent& event);

	//(*Identifiers(ImagePanel)
	//*)

	//(*Declarations(ImagePanel)
	//*)
	/**< origin iamge, not alter. */
	wxImage m_imgOrigin;
	/**< image to display. */
	wxImage m_img;
	/**< note the old size of the panel, used in calculate locate when size changed */
	wxSize m_szClient = { -1, -1};

	/**< image zoom scale */
	wxDouble m_dScale = 1.0;
	/**< zoom percentage */
	wxDouble m_dScaleDelta = 0.5;
	/**< image rect to draw */
	wxRect2DDouble m_rcSrc = {0.0, 0.0, 0.0, 0.0};
	/**< draw rect on wnd */
	wxRect2DDouble m_rcDest = {0.0, 0.0, 0.0, 0.0};

	/**< memorydc for double buffer draw */
	wxMemoryDC m_dcMem;

    /**< param for mouse */
    struct _ST_MOUSE_PARAM
    {
        /** \brief mouse state flag
         *
         *  0:none 1:LD 2:drag
         *
         */
         wxInt32 iState = 0;
        /** \brief mouse function flag
         *
         *  0:none 1:image move 2:zoonrect
         *
         */
        wxInt32 iFuc = 0;
        /**< start point */
        wxPoint ptB;
        /**< end point */
        wxPoint ptE;

        /**< the regular rect of mouse sel */
        wxRect rcSel;
        /**< the size of mouse move */
        wxSize szMv;
    } m_stMP;

	DECLARE_EVENT_TABLE()
};

#endif