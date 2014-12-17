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
#pragma once
#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

//(*Headers(ImagePanel)
//*)
#include <wx/panel.h>
#include <wx/geometry.h>
#include <wx/dcmemory.h>

/** \brief image panel's cmd
 *
 * cmd offered by the panel
 * used in update ui or function mark
 *
 */
enum class IMGPL_CMD : int
{
    NONE,

    IMG_ZIN,
    IMG_ZOUT,
    IMG_ZRECT,		// wxRect: xy-lefttop w-width h-height
    IMG_ZFIT,
    IMG_ZACTUAL,
    IMG_MOVE,		// wxSize: x-width y-height

    SEL_CIRCLE,		// wxRect: xy-center w-radius h-radius
    SEL_ELLIPSE,	// wxRect: xy-lefttop w-width h-height
    SEL_SQUARE,		// wxRect: xy-lefttop wh-length
    SEL_RECTANGLE	// wxRect: xy-lefttop w-width h-height
};

/** \brief declare notify evet type
 *
 * notify event for notify parent wnd
 * event's sublte kind is the m_emCMD, access by SetCMD or GetCMD
 * event's param is m_pParam, cacessed by SetParam or GetParam
 *
 */
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_IMGPL, 100)
END_DECLARE_EVENT_TYPES()
class wxImgplEvent : public wxNotifyEvent
{
	/**< event's sublte kind */
	IMGPL_CMD m_emCMD = IMGPL_CMD::NONE;
	/**< event's param, sepcific depend kind */
	void* m_pParam = nullptr;
public:
	wxImgplEvent(wxEventType commandType = wxEVT_IMGPL, int winid = 0)
		: wxNotifyEvent(commandType, winid)
	{
	}
	wxImgplEvent(wxImgplEvent& event)
		: wxNotifyEvent(event)
		, m_emCMD(event.m_emCMD)
		, m_pParam(event.m_pParam)
	{
	}
	virtual wxEvent* Clone()
	{
		return new wxImgplEvent(*this);
	}

	inline void SetCMD(IMGPL_CMD cmd)
	{
		m_emCMD = cmd;
	}
	inline IMGPL_CMD GetCMD()
	{
		return m_emCMD;
	}
	inline void SetParam(void* p)
	{
		m_pParam = p;
	}
	inline void* GetParam()
	{
		return m_pParam;
	}

private:
	DECLARE_DYNAMIC_CLASS(wxImgplEvent)
};
typedef void (wxEvtHandler::*wxImgplEventFunction)(wxImgplEvent& event);
#define EVT_IMGPL(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_IMGPL, id, -1, (wxObjectEventFunction)(wxImgplEventFunction)fn, (wxObject*)nullptr),


/** \brief image panel
 *
 * dislay image, and provide some image display control functions
 * such as zoom, move, et al.
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
	bool IsOK()
	{
		return m_img.IsOk();
	}

	/** \brief set image to dispaly
	 *
	 * \param img wxImage&	[IN] seted image
	 * \return bool			true:success false:failed
	 *
	 */
	bool SetImg(wxImage& img);
	/** \brief destroy image
	 *
	 * \return bool true:success false:failed
	 *
	 */
	bool ReleaseImg();

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
	/** \brief according to the specified zoom percentage
	 *
	 * \param wxDouble dScale              [IN] the specified zoom percentage
	 * \param wxPoint* pPt = nullptr    [IN] the fixed point in wnd coordinate, if nullptr, use the wnd's center
	 * \return bool true:success false:failed
	 *
	 */
	bool ImgZoomScale(wxDouble dScale, wxPoint* pPt = nullptr);
	/** \brief mouse fuc is sel the display rect
	 *
	 * \see bool ImgZoomRect(wxRect rcSel)
	 * \return bool true:success false:failed
	 *
	 */
	bool ImgZoomRect();
	/** \brief display image of sel rect full fit wnd
	 *
	 * \param wxRect rc	[IN] rect baseed of wnd
	 * \return bool true:success false:failed
	 *
	 */
	bool ImgZoomRect(wxRect rcSel);
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
	/** \brief move image
	 *
	 * \param wxSize szMove	[IN] the size to move in wnd coordinate
	 * \return bool true:success false:failed
	 *
	 */
	bool ImgMove(wxSize szMove);

	/** \brief select a CIRCLE/ELLIPSE/SQUARE/RECTANGLE in image
	 *
	 * \param const wxString& imgFile	[IN] cursor image file(png), if "", cursor_cross be used
	 * \param int hx				[IN] hotspot x-coordinate
	 * \param int hy				[IN] hotspot y-coordinate
	 * \return bool true:success false:failed
	 *
	 */
	bool SelTools(IMGPL_CMD cmd, const wxString& imgFile = wxEmptyString, int hx = 0, int hy = 0);

	/** \brief proc func btn ui update
	 *
	 * \param IMGPL_CMD cmd					[IN] cmd type
	 * \param event wxUpdateUIEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool UpdateUI(IMGPL_CMD cmd, wxUpdateUIEvent& event);

	/** \brief view coord to image coord
	 *
	 * \param wxPoint& pt	[INOUT]
	 * \param wxSize& sz		[INOUT]
	 * \return wxRect& rc		[INOUT]
	 *
	 */
	bool V2I(wxPoint& pt);
	bool V2I(wxSize& sz);
	bool V2I(wxRect& rc);
	/** \brief image coord to view coord
	 *
	 * \param wxPoint& pt	[INOUT]
	 * \param wxSize& sz		[INOUT]
	 * \return wxRect& rc		[INOUT]
	 *
	 */
	bool I2V(wxPoint& pt);
	bool I2V(wxSize& sz);
	bool I2V(wxRect& rc);

protected:

private:
	/** \brief calculate the sel max size, related to center
	 *
	 * \param bool bEqual	[IN] true:min(w, h) to be w and h |false:w is w, h is h
	 * \return bool true:success false:failed
	 *
	 */
	bool CalcuMaxSelSize(bool bEqual = false);
	/** \brief calculate the rect of the mouse sel
	 *
	 * \param bool bI		[IN] true:the result convert to image coord |false: retain in view coord
	 * \param bool bEqual	[IN] true:min(w, h) to be w and h |false:w is w, h is h
	 * \param bool bCenter	[IN] true:begin point is the center |false:begin-point is left-top
	 * \return bool true:success false:failed
	 *
	 */
	bool RegulaSelRect(bool bI = false, bool bEqual = false, bool bCenter = false);
	/** \brief calculate the sel radus
	 *
	 * \param bool bI	[IN] true:the result convert to image coord |false: retain in view coord
	 * \return bool true:success false:failed
	 *
	 */
	bool CalcuSelRadius(bool bI = false);

	/** \brief Image Move Mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDImgMove(wxMouseEvent& event);
	bool MLUImgMove(wxMouseEvent& event);
	bool MMVImgMove(wxMouseEvent& event);
	/** \brief Image zoom rect mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDImgZRect(wxMouseEvent& event);
	bool MLUImgZRect(wxMouseEvent& event);
	bool MMVImgZRect(wxMouseEvent& event);
	/** \brief Select a CIRCLE mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDSelCircle(wxMouseEvent& event);
	bool MLUSelCircle(wxMouseEvent& event);
	bool MMVSelCircle(wxMouseEvent& event);
	/** \brief Select a ELLIPSE mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDSelEllipse(wxMouseEvent& event);
	bool MLUSelEllipse(wxMouseEvent& event);
	bool MMVSelEllipse(wxMouseEvent& event);
	/** \brief Select a SQUARE mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDSelSquare(wxMouseEvent& event);
	bool MLUSelSquare(wxMouseEvent& event);
	bool MMVSelSquare(wxMouseEvent& event);
	/** \brief Select a RECTANGLE mouse event
	 *
	 * \param event wxMouseEvent& [INOUT] event
	 * \return bool true:success false:failed
	 *
	 */
	bool MLDSelRectangle(wxMouseEvent& event);
	bool MLUSelRectangle(wxMouseEvent& event);
	bool MMVSelRectangle(wxMouseEvent& event);

	/** \brief stop drag
	 *
	 * \param true bool bFuc=true [IN] true:remain the fuc false:reset the func
	 * \return bool true:success false:failed
	 *
	 */
	bool EndDrag(bool bFuc = true);

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
	/** \brief context menu
	 *
	 * \param event wxContextMenuEvent&	[INOUT] the event
	 * \return void
	 *
	 */
	void OnContextMenu(wxContextMenuEvent& event);
	/** \brief kill focus. if capture mouse, should release
	 *
	 * \param event wxFocusEvent&	[INOUT] the event
	 * \return void
	 *
	 */
	void OnKillFocus(wxFocusEvent& event);
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

    /** \brief ContextMenu save image
     *
     * \param event wxCommandEvent&   [INOUT] the event
     * \return void
     *
     */
	void OnCmenuSave(wxCommandEvent& event);


	// ContextMenu
	static const long ID_CMENU_SAVE;


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
		 *  \see IMGPL_CMD
		 *
		 */
		IMGPL_CMD emFuc = IMGPL_CMD::NONE;
		/**< start point */
		wxPoint ptB;
		/**< end point */
		wxPoint ptE;

		/**< the regular rect of mouse sel */
		wxRect rcSel;
		/**< the size of mouse move */
		wxSize szMv;

		/**< remain the dest rect before move image */
		wxRect2DDouble rcDO;
		/**< remain the src rect before move iamge */
		wxRect2DDouble rcSO;
	} m_stMP;

	DECLARE_EVENT_TABLE()
};

#endif
