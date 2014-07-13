#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/filename.h>

#include <math.h>
#include "ImagePanel.h"

// ContextMenu
const long ImagePanel::ID_CMENU_SAVE = wxNewId();

// define notify evet type
IMPLEMENT_DYNAMIC_CLASS(wxImgplEvent, wxNotifyEvent)
DEFINE_EVENT_TYPE(wxEVT_IMGPL)


BEGIN_EVENT_TABLE(ImagePanel,wxPanel)
END_EVENT_TABLE()

ImagePanel::ImagePanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& sz)
{
	// create panel
	Create(parent, id, pos, sz, wxTAB_TRAVERSAL, _T("_imgpanel"));
	// stop auto erase background
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	// memorydc for double buffer draw
	wxInt32 iScrW = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, this);
	wxInt32 iScrH = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, this);
	wxBitmap bmp(iScrW, iScrH);
	m_dcMem.SelectObject(bmp);
	m_dcMem.SetBackground(wxBrush(wxColor(0x00606060)));
	m_dcMem.Clear();

	// Event process
	Connect(wxEVT_PAINT, (wxObjectEventFunction)&ImagePanel::OnPaint);
	Connect(wxEVT_ERASE_BACKGROUND, (wxObjectEventFunction)&ImagePanel::OnErase);
	Connect(wxEVT_SIZE, (wxObjectEventFunction)&ImagePanel::OnSize);
	Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ImagePanel::OnContextMenu);
	Connect(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&ImagePanel::OnKillFocus);
	// mouse event
	Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ImagePanel::OnMouseLD);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ImagePanel::OnMouseLU);
	Connect(wxEVT_MOTION, (wxObjectEventFunction)&ImagePanel::OnMouseMove);
	// menu or tool-button command
	Connect(ID_CMENU_SAVE, wxEVT_MENU, (wxObjectEventFunction)&ImagePanel::OnCmenuSave);
}

ImagePanel::~ImagePanel()
{
	// release double buffer draw
	m_dcMem.SelectObject(wxNullBitmap);
}

////////////////////////////////////////////////////////////////////////////////////////
/**< set the iamge to dispaly */
bool ImagePanel::SetImg(wxImage& img)
{
	// whether the new image can be used
	if (!img.IsOk())
		return false;
	// reference the image used to display.
	if (m_img.IsOk())
		m_img.Destroy();
	m_img = img;

	// calculate display params
	m_szClient = GetClientSize();
	wxSize szImg = m_img.GetSize();
	// if wnd's size greate then img's size, image disp as its actual size
	if (m_szClient.GetWidth() > szImg.GetWidth() && m_szClient.GetHeight() > szImg.GetHeight() )
		ImgZoomActual();
	else
		ImgZoomFit();

	return true;
}

/**< destroy image */
bool ImagePanel::ReleaseImg()
{
	// stop drag and mouse func
	EndDrag(false);

	// destroy image
	if (m_img.IsOk())
		m_img.Destroy();
	// update ui
	Refresh(false);

	return true;
}

/**< zoomin image */
bool ImagePanel::ImgZoomIn()
{
	// new scale
	wxDouble dScale = m_dScale * (1 + m_dScaleDelta);
	ImgZoomScale(dScale);

	return true;
}

/**< zoomout image */
bool ImagePanel::ImgZoomOut()
{
	// new scale
	wxDouble dScale = m_dScale * m_dScaleDelta;
	ImgZoomScale(dScale);

	return true;
}

/**< according to the specified zoom percentage */
bool ImagePanel::ImgZoomScale(wxDouble dScale, wxPoint* pPt /*= nullptr*/)
{
	if (!m_img.IsOk())
		return false;
	if (dScale <= 0.0)
		return false;
	if (dScale > 20.0)
		dScale = 20.0;

	wxRect2DDouble rcWnd(0.0, 0.0, 0.0, 0.0);		// wnd rect
	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// img rect
	do
	{
		wxSize szWnd = GetClientSize();
		wxSize szImg = m_img.GetSize();
		rcWnd.m_width = szWnd.GetWidth();
		rcWnd.m_height = szWnd.GetHeight();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// default image pix in wnd center as the fixed point
	wxPoint2DDouble ptV0, ptI0;
	wxDouble dTmp = 0.0;
	if (pPt == nullptr)
	{
		ptV0.m_x = rcWnd.m_width/2.0;
		ptV0.m_y = rcWnd.m_height/2.0;
	}
	else
	{
		ptV0.m_x = pPt->x;
		ptV0.m_y = pPt->y;
	}
	dTmp = (ptV0.m_x - m_rcDest.m_x)/m_dScale;	// V2I
	ptI0.m_x = m_rcSrc.m_x + dTmp;
	dTmp = (ptV0.m_y - m_rcDest.m_y)/m_dScale;
	ptI0.m_y = m_rcSrc.m_y + dTmp;

	// the image view rect in new scale
	wxRect2DDouble rcNew;
	rcNew.m_x = ptV0.m_x - ptI0.m_x*dScale;
	rcNew.m_y = ptV0.m_y - ptI0.m_y*dScale;
	rcNew.m_width = rcImg.m_width*dScale;
	rcNew.m_height = rcImg.m_height*dScale;

	// Dest rect is the intersection of new image view rect & wnd rect
	wxRect2DDouble::Intersect(rcWnd, rcNew, &m_rcDest);

	// the src rect in new scale
	m_rcSrc.m_x = ptI0.m_x + (m_rcDest.m_x - ptV0.m_x)/dScale;
	m_rcSrc.m_y = ptI0.m_y + (m_rcDest.m_y - ptV0.m_y)/dScale;
	m_rcSrc.m_width = m_rcDest.m_width/dScale;
	m_rcSrc.m_height = m_rcDest.m_height/dScale;
	// ensure the src rect is valid
	if (m_rcSrc.m_x < 0.0)
		m_rcSrc.m_x = 0.0;
	if (m_rcSrc.m_y < 0.0)
		m_rcSrc.m_y = 0.0;
	if (m_rcSrc.GetRight() > rcImg.m_width)
		m_rcSrc.m_width = rcImg.m_width - m_rcSrc.m_x;
	if (m_rcSrc.GetBottom() > rcImg.m_height)
		m_rcSrc.m_height = rcImg.m_height - m_rcSrc.m_y;

	// new zoom scale
	m_dScale = dScale;

	// update panel
	Refresh(false);

	return true;
}

/**< sel the display rect*/
bool ImagePanel::ImgZoomRect()
{
	if (!m_img.IsOk())
		return false;

	if (m_stMP.emFuc != IMGPL_CMD::IMG_ZRECT)
	{
		// mouse function is image zoom rect
		m_stMP.emFuc = IMGPL_CMD::IMG_ZRECT;
		// set mouse cur
		SetCursor(wxCursor(_("./skin/ZoomRect.cur"), wxBITMAP_TYPE_CUR));
	}
	else
	{
		// mouse function is none
		m_stMP.emFuc = IMGPL_CMD::NONE;
		// set mouse cur
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}

	return true;
}

/**< display image of sel rect full fit wnd */
bool ImagePanel::ImgZoomRect(wxRect rcSel)
{
	if (!m_img.IsOk())
		return false;
	// the rect must be regular
	if (rcSel.width <= 2 || rcSel.height < 2)
		return false;

	wxRect2DDouble rcWnd(0.0, 0.0, 0.0, 0.0);		// wnd rect
	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// image rect
	do
	{
		wxSize szWnd = GetClientSize();
		wxSize szImg = m_img.GetSize();
		rcWnd.m_width = szWnd.GetWidth();
		rcWnd.m_height = szWnd.GetHeight();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// calculate the new scale
	wxDouble dScale = 1.0;
	if (rcWnd.m_width/rcWnd.m_height > rcSel.width/(wxDouble)rcSel.height)
		dScale = rcWnd.m_height/rcSel.height;
	else
		dScale = rcWnd.m_width/rcSel.width;
	dScale *= m_dScale;
	if (dScale > 20.0)
		dScale = 20.0;

	// sel rect center point
	wxPoint2DDouble ptV0, ptI0;
	ptV0.m_x = rcSel.x + rcSel.width/2.0;
	ptV0.m_y = rcSel.y + rcSel.height/2.0;
	ptI0.m_x = m_rcSrc.m_x + (ptV0.m_x - m_rcDest.m_x)/m_dScale;
	ptI0.m_y = m_rcSrc.m_y + (ptV0.m_y - m_rcDest.m_y)/m_dScale;

	// calculate the new image rect in wnd coordinate
	wxRect2DDouble rcNew;
	rcNew.m_x = ptV0.m_x - ptI0.m_x*dScale;
	rcNew.m_y = ptV0.m_y - ptI0.m_y*dScale;
	rcNew.m_width = rcImg.m_width*dScale;
	rcNew.m_height = rcImg.m_height*dScale;
	// move the sel rect center to the wnd center
	rcNew.m_x -= ptV0.m_x - rcWnd.m_width/2.0;
	rcNew.m_y -= ptV0.m_y - rcWnd.m_height/2.0;
	ptV0.m_x = rcWnd.m_width/2.0;
	ptV0.m_y = rcWnd.m_height/2.0;

	// the dest rect is the intersection of the new image view rect & wnd rect
	wxRect2DDouble::Intersect(rcWnd, rcNew, &m_rcDest);

	// calculate the src rect
	m_rcSrc.m_x = ptI0.m_x + (m_rcDest.m_x - ptV0.m_x)/dScale;
	m_rcSrc.m_y = ptI0.m_y + (m_rcDest.m_y - ptV0.m_y)/dScale;
	m_rcSrc.m_width = m_rcDest.m_width/dScale;
	m_rcSrc.m_height = m_rcDest.m_height/dScale;
	// ensure the src rect is valid
	if (m_rcSrc.m_x < 0.0)
		m_rcSrc.m_x = 0.0;
	if (m_rcSrc.m_y < 0.0)
		m_rcSrc.m_y = 0.0;
	if (m_rcSrc.GetRight() > rcImg.m_width)
		m_rcSrc.m_width = rcImg.m_width - m_rcSrc.m_x;
	if (m_rcSrc.GetBottom() > rcImg.m_height)
		m_rcSrc.m_height = rcImg.m_height - m_rcSrc.m_y;

	// the new scale
	m_dScale = dScale;

	// update ui
	Refresh(false);

	return true;
}

/**< dispaly image in real size */
bool ImagePanel::ImgZoomActual()
{
	if (!m_img.IsOk())
		return false;

	wxRect2DDouble rcWnd(0.0, 0.0, 0.0, 0.0);		// wnd rect
	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// img rect
	do
	{
		wxSize szWnd = GetClientSize();
		wxSize szImg = m_img.GetSize();
		rcWnd.m_width = szWnd.GetWidth();
		rcWnd.m_height = szWnd.GetHeight();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// image is actual size displayed
	m_dScale = 1.0;

	// Horizontal positioning
	if (rcWnd.m_width > rcImg.m_width)
	{
		m_rcSrc.m_x = 0.0;
		m_rcSrc.m_width = rcImg.m_width;
		// wnd left and right have space
		m_rcDest.m_x = (rcWnd.m_width - rcImg.m_width)/2.0;
		m_rcDest.m_width = rcImg.m_width;
	}
	else
	{
		// only part of image be displayed
		m_rcSrc.m_x = (rcImg.m_width - rcWnd.m_width)/2.0;
		m_rcSrc.m_width = rcWnd.m_width;
		// wnd is filled width
		m_rcDest.m_x = 0.0;
		m_rcDest.m_width = rcWnd.m_width;
	}
	// Vertical positioning
	if (rcWnd.m_height > rcImg.m_height)
	{
		m_rcSrc.m_y = 0.0;
		m_rcSrc.m_height = rcImg.m_height;
		// wnd's top and bottom have space
		m_rcDest.m_y = (rcWnd.m_height - rcImg.m_height)/2.0;
		m_rcDest.m_height = rcImg.m_height;
	}
	else
	{
		// only part image be displayed
		m_rcSrc.m_y = (rcImg.m_height - rcWnd.m_height)/2.0;
		m_rcSrc.m_height = rcWnd.m_height;
		// wnd is fill height
		m_rcDest.m_y = 0.0;
		m_rcDest.m_height = rcWnd.m_height;
	}

	// update panel
	Refresh(false);

	return true;
}

/**< display image full fit wnd */
bool ImagePanel::ImgZoomFit()
{
	if (!m_img.IsOk())
		return false;

	wxRect2DDouble rcWnd(0.0, 0.0, 0.0, 0.0);		// wnd rect
	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// img rect
	do
	{
		wxSize szWnd = GetClientSize();
		wxSize szImg = m_img.GetSize();
		rcWnd.m_width = szWnd.GetWidth();
		rcWnd.m_height = szWnd.GetHeight();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// image is full displayed
	m_rcSrc.m_x = 0.0;
	m_rcSrc.m_y = 0.0;
	m_rcSrc.m_width = rcImg.m_width;
	m_rcSrc.m_height = rcImg.m_height;

	// calculation scale and dest rect
	if (rcWnd.m_width/rcWnd.m_height > rcImg.m_width/rcImg.m_height)	// wnd is wide then image, or image is heigher then wnd
	{
		wxDouble dTmp = 0.0;
		m_dScale = rcWnd.m_height/rcImg.m_height;
		dTmp = rcImg.m_width*m_dScale;	// image scale width

		m_rcDest.m_x = (rcWnd.m_width - dTmp)/2.0;
		m_rcDest.m_y = 0.0;
		m_rcDest.m_width = dTmp;
		m_rcDest.m_height = rcWnd.m_height;
	}
	else	// wnd is heigher than image, or image is wide than wnd
	{
		wxDouble dTmp = 0.0;
		m_dScale = rcWnd.m_width/rcImg.m_width;
		dTmp = rcImg.m_height*m_dScale;	// image scale height

		m_rcDest.m_x = 0.0;
		m_rcDest.m_y = (rcWnd.m_height - dTmp)/2.0;
		m_rcDest.m_width = rcWnd.m_width;
		m_rcDest.m_height = dTmp;
	}

	// update panel
	Refresh(false);

	return true;
}

/**< mouse fuc is move image */
bool ImagePanel::ImgMove()
{
	if (!m_img.IsOk())
		return false;

	if (m_stMP.emFuc != IMGPL_CMD::IMG_MOVE)
	{
		// mouse function is image move
		m_stMP.emFuc = IMGPL_CMD::IMG_MOVE;
		// set mouse cur
		SetCursor(wxCursor(_("./skin/HandOpen.cur"), wxBITMAP_TYPE_CUR));
	}
	else
	{
		// mouse function is none
		m_stMP.emFuc = IMGPL_CMD::NONE;
		// set mouse cur
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}

	return true;
}

/**< move image */
bool ImagePanel::ImgMove(wxSize szMove)
{
	if (!m_img.IsOk())
		return false;
	// the move size must be valid
	if (szMove.x == 0 || szMove.y == 0)
		return false;

	wxPoint2DDouble ptMove(szMove.x, szMove.y);

	wxRect2DDouble rcWnd(0.0, 0.0, 0.0, 0.0);		// wnd rect
	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// img rect
	do
	{
		wxSize szWnd = GetClientSize();
		wxSize szImg = m_img.GetSize();
		rcWnd.m_width = szWnd.GetWidth();
		rcWnd.m_height = szWnd.GetHeight();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// current image rect in wnd coordinate
	wxRect2DDouble rcNew;
	rcNew.m_x = m_rcDest.m_x - m_rcSrc.m_x*m_dScale;
	rcNew.m_y = m_rcDest.m_y - m_rcSrc.m_y*m_dScale;
	rcNew.m_width = rcImg.m_width*m_dScale;
	rcNew.m_height = rcImg.m_height*m_dScale;
	// then horizontal move the rect
	if ((rcNew.m_x + ptMove.m_x) > rcWnd.m_width/2.0)
		ptMove.m_x = (int)(rcWnd.m_width/2.0 - rcNew.m_x);
	else if ((rcNew.GetRight() + ptMove.m_x) <= rcWnd.m_width/2.0)
		ptMove.m_x = (int)(rcWnd.m_width/2.0 - rcNew.GetRight());
	// and then vertical move the rect
	if ((rcNew.m_y + ptMove.m_y) > rcWnd.m_height/2.0)
		ptMove.m_y = (int)(rcWnd.m_height/2.0 - rcNew.m_y);
	else if ((rcNew.GetBottom() + ptMove.m_y) <= rcWnd.m_height/2.0)
		ptMove.m_y = (int)(rcWnd.m_height/2.0 - rcNew.GetBottom());
	rcNew.Offset(ptMove);

	// the dest rect is the intersection of new rect  and wnd rect
	wxRect2DDouble::Intersect(rcNew, rcWnd, &m_rcDest);

	// calculate the src rect
	m_rcSrc.m_x = (m_rcDest.m_x - rcNew.m_x)/m_dScale;
	m_rcSrc.m_y = (m_rcDest.m_y - rcNew.m_y)/m_dScale;
	m_rcSrc.m_width = m_rcDest.m_width/m_dScale;
	m_rcSrc.m_height = m_rcDest.m_height/m_dScale;
	// ensure the src rect is valid
	if (m_rcSrc.m_x < 0.0)
		m_rcSrc.m_x = 0.0;
	if (m_rcSrc.m_y < 0.0)
		m_rcSrc.m_y = 0.0;
	if (m_rcSrc.GetRight() > rcImg.m_width)
		m_rcSrc.m_width = rcImg.m_width - m_rcSrc.m_x;
	if (m_rcSrc.GetBottom() > rcImg.m_height)
		m_rcSrc.m_height = rcImg.m_height - m_rcSrc.m_y;

	// update panel
	Refresh(false);

	return true;
}

/**< select faint spot in image */
bool ImagePanel::SelFaint()
{
	if (!m_img.IsOk())
		return false;

	if (m_stMP.emFuc != IMGPL_CMD::SEL_FAINT)
	{
		// mouse function is sel faint spot
		m_stMP.emFuc = IMGPL_CMD::SEL_FAINT;
		// set mouse cur
		wxImage img(_T("./skin/SelFaint.png"), wxBITMAP_TYPE_PNG);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		SetCursor(wxCursor(img));
	}
	else
	{
		// mouse function is none
		m_stMP.emFuc = IMGPL_CMD::NONE;
		// set mouse cur
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}

	return true;
}

/**< select min spot in image */
bool ImagePanel::SelMin()
{
	if (!m_img.IsOk())
		return false;

	if (m_stMP.emFuc != IMGPL_CMD::SEL_MIN)
	{
		// mouse function is sel min radius spot
		m_stMP.emFuc = IMGPL_CMD::SEL_MIN;
		// set mouse cur
		wxImage img(_T("./skin/SelMin.png"), wxBITMAP_TYPE_PNG);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		SetCursor(wxCursor(img));
	}
	else
	{
		// mouse function is none
		m_stMP.emFuc = IMGPL_CMD::NONE;
		// set mouse cur
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}

	return true;
}

/**< select max spot in image */
bool ImagePanel::SelMax()
{
	if (!m_img.IsOk())
		return false;

	if (m_stMP.emFuc != IMGPL_CMD::SEL_MAX)
	{
		// mouse function is sel max radius spot
		m_stMP.emFuc = IMGPL_CMD::SEL_MAX;
		// set mouse cur
		wxImage img(_T("./skin/SelMax.png"), wxBITMAP_TYPE_PNG);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
		img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		SetCursor(wxCursor(img));
	}
	else
	{
		// mouse function is none
		m_stMP.emFuc = IMGPL_CMD::NONE;
		// set mouse cur
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}

	return true;
}

/**< proc func btn ui update */
bool ImagePanel::UpdateUI(IMGPL_CMD cmd, wxUpdateUIEvent& event)
{
	switch (cmd)
	{
	case IMGPL_CMD::IMG_ZIN:
	case IMGPL_CMD::IMG_ZOUT:
	case IMGPL_CMD::IMG_ZFIT:
	case IMGPL_CMD::IMG_ZACTUAL:
		event.Enable(m_img.IsOk());
		break;
	case IMGPL_CMD::IMG_MOVE:
	case IMGPL_CMD::IMG_ZRECT:
	case IMGPL_CMD::SEL_FAINT:
	case IMGPL_CMD::SEL_MIN:
	case IMGPL_CMD::SEL_MAX:
		event.Enable(m_img.IsOk());
		event.Check(m_stMP.emFuc == cmd);
		break;
	default:
		break;
	}
	return true;
}

/**< view coord to image coord */
bool ImagePanel::V2I(wxPoint& pt)
{
	if (!m_img.IsOk())
		return false;
	pt.x = (int)(m_rcSrc.m_x + (pt.x - m_rcDest.m_x)/m_dScale);
	pt.y = (int)(m_rcSrc.m_y + (pt.y - m_rcDest.m_y)/m_dScale);
	return true;
}
bool ImagePanel::V2I(wxSize& sz)
{
	if (!m_img.IsOk())
		return false;
	sz.x = (int)(sz.x/m_dScale);
	sz.y = (int)(sz.y/m_dScale);
	return true;
}
bool ImagePanel::V2I(wxRect& rc)
{
	if (!m_img.IsOk())
		return false;
	rc.x = (int)(m_rcSrc.m_x + (rc.x - m_rcDest.m_x)/m_dScale);
	rc.y = (int)(m_rcSrc.m_y + (rc.y - m_rcDest.m_y)/m_dScale);
	rc.width = (int)(rc.width/m_dScale);
	rc.height = (int)(rc.height/m_dScale);
	return true;
}
/**< image coord to view coord */
bool ImagePanel::I2V(wxPoint& pt)
{
	if (!m_img.IsOk())
		return false;
	pt.x = (int)(m_rcDest.m_x + (pt.x - m_rcSrc.m_x)*m_dScale);
	pt.y = (int)(m_rcDest.m_y + (pt.y - m_rcSrc.m_y)*m_dScale);
	return true;
}
bool ImagePanel::I2V(wxSize& sz)
{
	if (!m_img.IsOk())
		return false;
	sz.x = (int)(sz.x*m_dScale);
	sz.y = (int)(sz.y*m_dScale);
	return true;
}
bool ImagePanel::I2V(wxRect& rc)
{
	if (!m_img.IsOk())
		return false;
	rc.x = (int)(m_rcDest.m_x + (rc.x - m_rcSrc.m_x)*m_dScale);
	rc.y = (int)(m_rcDest.m_y + (rc.y - m_rcSrc.m_y)*m_dScale);
	rc.width = (int)(rc.width*m_dScale);
	rc.height = (int)(rc.height*m_dScale);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
/**< calculate the rect of the mouse sel */
bool ImagePanel::RegulaSelRect()
{
	// calculate the regula sel rect
	if (m_stMP.ptB.x < m_stMP.ptE.x)
	{
		m_stMP.rcSel.x = m_stMP.ptB.x;
		m_stMP.rcSel.width = m_stMP.ptE.x - m_stMP.ptB.x;
	}
	else
	{
		m_stMP.rcSel.x = m_stMP.ptE.x;
		m_stMP.rcSel.width = m_stMP.ptB.x - m_stMP.ptE.x;
	}
	if (m_stMP.ptB.y < m_stMP.ptE.y)
	{
		m_stMP.rcSel.y = m_stMP.ptB.y;
		m_stMP.rcSel.height = m_stMP.ptE.y - m_stMP.ptB.y;
	}
	else
	{
		m_stMP.rcSel.y = m_stMP.ptE.y;
		m_stMP.rcSel.height = m_stMP.ptB.y - m_stMP.ptE.y;
	}
	wxRect rcDest((int)m_rcDest.m_x, (int)m_rcDest.m_y, (int)m_rcDest.m_width, (int)m_rcDest.m_height);
	m_stMP.rcSel.Intersect(rcDest);
	m_stMP.rcSel.x --;		// don't know why, it offset right 1px

	return true;
}

/**< calculate the max radius of sel circle */
bool ImagePanel::CalcuMaxRadius()
{
	// the distance to 4 boders
	m_stMP.rcSel.x =m_stMP.ptB.x - (int)m_rcDest.m_x;
	m_stMP.rcSel.y = m_stMP.ptB.y - (int)m_rcDest.m_y;
	m_stMP.rcSel.width = m_rcDest.GetRight() - 1 - m_stMP.ptB.x;
	m_stMP.rcSel.height = m_rcDest.GetBottom() - 1 - m_stMP.ptB.y;
	// the max distance
	int iMaxRad = m_stMP.rcSel.x;
	if (iMaxRad > m_stMP.rcSel.y)
		iMaxRad = m_stMP.rcSel.y;
	if (iMaxRad > m_stMP.rcSel.width)
		iMaxRad = m_stMP.rcSel.width;
	if (iMaxRad > m_stMP.rcSel.height)
		iMaxRad = m_stMP.rcSel.height;

	// the initialize result
	m_stMP.rcSel.x = m_stMP.ptB.x;		// the center
	m_stMP.rcSel.y = m_stMP.ptB.y;
	m_stMP.rcSel.width = 0;			// the radius
	m_stMP.rcSel.height = iMaxRad;	// the maxradius

	return true;
}

/**< calculate the sel radus */
bool ImagePanel::CalcuSelRadius(bool bI /*=false*/)
{
	int cx = m_stMP.ptE.x - m_stMP.ptB.x;
	int cy = m_stMP.ptE.y - m_stMP.ptB.y;
	m_stMP.rcSel.width = (int)sqrt(cx*cx + cy*cy);
	// cx should less than the max radius
	if (m_stMP.rcSel.width > m_stMP.rcSel.height)
		m_stMP.rcSel.width = m_stMP.rcSel.height;

	m_stMP.rcSel.x = m_stMP.ptB.x;		// the center
	m_stMP.rcSel.y = m_stMP.ptB.y;
	if (bI)
		V2I(m_stMP.rcSel);

	return true;
}

/**< Image Move Mouse event */
bool ImagePanel::MLDImgMove(wxMouseEvent& event)
{
	// flag
	m_stMP.iState = 1;              // button down
	m_stMP.ptB.x = event.m_x;   // start point
	m_stMP.ptB.y = event.m_y;
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;

	m_stMP.szMv.x = 0;
	m_stMP.szMv.y = 0;
	// cursor
	SetCursor(wxCursor(_T("./skin/HandClose.cur"), wxBITMAP_TYPE_CUR));
	// capture mouse
	CaptureMouse();

	return true;
}
bool ImagePanel::MLUImgMove(wxMouseEvent& event)
{
	if (m_stMP.iState == 2)
	{
		m_stMP.iState = 0;

		// new point & move incremention
		m_stMP.ptE.x = event.m_x;
		m_stMP.ptE.y = event.m_y;
		m_stMP.szMv.x = m_stMP.ptE.x - m_stMP.ptB.x;
		m_stMP.szMv.y = m_stMP.ptE.y - m_stMP.ptB.y;
		// Notify Parent: IMG_MOVE
		wxImgplEvent evt(wxEVT_IMGPL, GetId());
		evt.SetCMD(IMGPL_CMD::IMG_MOVE);
		evt.SetParam((void*)&m_stMP.szMv);
		GetParent()->GetEventHandler()->ProcessEvent(evt);
		// move the image
		m_rcDest = m_stMP.rcDO;
		m_rcSrc = m_stMP.rcSO;
		ImgMove(m_stMP.szMv);
	}
	m_stMP.iState = 0;
	// cursor
	SetCursor(wxCursor(_T("./skin/HandOpen.cur"), wxBITMAP_TYPE_CUR));
	// release mouse
	ReleaseMouse();

	return true;
}
bool ImagePanel::MMVImgMove(wxMouseEvent& event)
{
	if (m_stMP.iState == 1)
	{
		// switch to drag
		m_stMP.iState = 2;
		// remain old dest and src rect
		m_stMP.rcDO = m_rcDest;
		m_stMP.rcSO = m_rcSrc;
	}
	// new point & move incremention
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;
	m_stMP.szMv.x = m_stMP.ptE.x - m_stMP.ptB.x;
	m_stMP.szMv.y = m_stMP.ptE.y - m_stMP.ptB.y;
	// move the image
	m_rcDest = m_stMP.rcDO;
	m_rcSrc = m_stMP.rcSO;
	ImgMove(m_stMP.szMv);

	return true;
}

/**< Image zoom rect mouse event */
bool ImagePanel::MLDImgZRect(wxMouseEvent& event)
{
	// flag
	m_stMP.iState = 1;              // button down
	m_stMP.ptB.x = event.m_x;   // start point
	m_stMP.ptB.y = event.m_y;
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;

	m_stMP.rcSel.x = 0;
	m_stMP.rcSel.y = 0;
	m_stMP.rcSel.width = 0;
	m_stMP.rcSel.height = 0;
	// capture mouse
	CaptureMouse();

	return true;
}
bool ImagePanel::MLUImgZRect(wxMouseEvent& event)
{
	if (m_stMP.iState == 2)
	{
		m_stMP.iState = 0;
		// new point
		m_stMP.ptE.x = event.m_x;
		m_stMP.ptE.y = event.m_y;
		// calculate the sel rect
		RegulaSelRect();
		// Notify Parent: IMG_ZRECT
		wxImgplEvent evt(wxEVT_IMGPL, GetId());
		evt.SetCMD(IMGPL_CMD::IMG_ZRECT);
		evt.SetParam((void*)&m_stMP.rcSel);
		GetParent()->GetEventHandler()->ProcessEvent(evt);
		// Zoom rect
		ImgZoomRect(m_stMP.rcSel);
	}
	else
	{
		m_stMP.iState = 0;
		// update ui
		Refresh(false);
	}
	// Release Mouse
	ReleaseMouse();

	return true;
}
bool ImagePanel::MMVImgZRect(wxMouseEvent& event)
{
	if (m_stMP.iState == 1)
		m_stMP.iState = 2;  // switch to drag
	// new point
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;
	// calculate the sel recty
	RegulaSelRect();
	// draw the sel rectangle
	Refresh(false);

	return true;
}

/**< Select faint spot mouse event */
bool ImagePanel::MLDSelFaint(wxMouseEvent& event)
{
	// flag
	m_stMP.iState = 1;              // button down
	m_stMP.ptB.x = event.m_x;   // start point
	m_stMP.ptB.y = event.m_y;
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;

	// calcu the max radius
	CalcuMaxRadius();

	// capture mouse
	CaptureMouse();

	return true;
}
bool ImagePanel::MLUSelFaint(wxMouseEvent& event)
{
	// Release Mouse
	ReleaseMouse();
	if (m_stMP.iState == 2)	// have draged
	{
		m_stMP.iState = 0;
		// new point
		m_stMP.ptE.x = event.m_x;
		m_stMP.ptE.y = event.m_y;
		// calculate the radius
		CalcuSelRadius(true);
		// Notify Parent: SEL_FAINT
		wxImgplEvent evt(wxEVT_IMGPL, GetId());
		evt.SetCMD(IMGPL_CMD::SEL_FAINT);
		evt.SetParam((void*)&m_stMP.rcSel);
		GetParent()->GetEventHandler()->ProcessEvent(evt);
	}
	else
		m_stMP.iState = 0;
	// update ui
	Refresh(false);

	return true;
}
bool ImagePanel::MMVSelFaint(wxMouseEvent& event)
{
	if (m_stMP.iState == 1)
		// switch to drag
		m_stMP.iState = 2;
	// new point & move incremention
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;
	// calculate the radius
	CalcuSelRadius();
	// draw the circle
	Refresh(false);

	return true;
}

/**< Select min spot mouse event */
bool ImagePanel::MLDSelMin(wxMouseEvent& event)
{
	// flag
	m_stMP.iState = 1;              // button down
	m_stMP.ptB.x = event.m_x;   // start point
	m_stMP.ptB.y = event.m_y;
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;

	// calculate the max radus
	CalcuMaxRadius();

	// capture mouse
	CaptureMouse();

	return true;
}
bool ImagePanel::MLUSelMin(wxMouseEvent& event)
{
	// Release Mouse
	ReleaseMouse();
	if (m_stMP.iState == 2)	// have draged
	{
		m_stMP.iState = 0;
		// new point
		m_stMP.ptE.x = event.m_x;
		m_stMP.ptE.y = event.m_y;
		// calculate the radius
		CalcuSelRadius(true);
		// Notify Parent: SEL_MIN
		wxImgplEvent evt(wxEVT_IMGPL, GetId());
		evt.SetCMD(IMGPL_CMD::SEL_MIN);
		evt.SetParam((void*)&m_stMP.rcSel);
		GetParent()->GetEventHandler()->ProcessEvent(evt);
	}
	else
		m_stMP.iState = 0;
	// update ui
	Refresh(false);

	return true;
}
bool ImagePanel::MMVSelMin(wxMouseEvent& event)
{
	if (m_stMP.iState == 1)
		// switch to drag
		m_stMP.iState = 2;
	// new point & move incremention
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;
	// calculate the radius
	CalcuSelRadius();
	// draw the circle
	Refresh(false);

	return true;
}

/**< Select max spot mouse event */
bool ImagePanel::MLDSelMax(wxMouseEvent& event)
{
	// flag
	m_stMP.iState = 1;              // button down
	m_stMP.ptB.x = event.m_x;   // start point
	m_stMP.ptB.y = event.m_y;
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;

	// calculate the max radus
	CalcuMaxRadius();

	// capture mouse
	CaptureMouse();

	return true;
}
bool ImagePanel::MLUSelMax(wxMouseEvent& event)
{
	// Release Mouse
	ReleaseMouse();
	if (m_stMP.iState == 2)	// have draged
	{
		m_stMP.iState = 0;
		// new point
		m_stMP.ptE.x = event.m_x;
		m_stMP.ptE.y = event.m_y;
		// calculate the radius
		CalcuSelRadius(true);
		// Notify Parent: SEL_MAX
		wxImgplEvent evt(wxEVT_IMGPL, GetId());
		evt.SetCMD(IMGPL_CMD::SEL_MAX);
		evt.SetParam((void*)&m_stMP.rcSel);
		GetParent()->GetEventHandler()->ProcessEvent(evt);
	}
	else
		m_stMP.iState = 0;
	// update ui
	Refresh(false);

	return true;
}
bool ImagePanel::MMVSelMax(wxMouseEvent& event)
{
	if (m_stMP.iState == 1)
		// switch to drag
		m_stMP.iState = 2;
	// new point & move incremention
	m_stMP.ptE.x = event.m_x;
	m_stMP.ptE.y = event.m_y;
	// calculate the radius
	CalcuSelRadius();
	// draw the circle
	Refresh(false);

	return true;
}

/**< stop drag */
bool ImagePanel::EndDrag(bool bFuc /*= true*/)
{
	switch(m_stMP.emFuc)
	{
	case IMGPL_CMD::IMG_MOVE:		// iamge move
	{
		if (m_stMP.iState != 0)
		{
			ReleaseMouse();
			m_stMP.iState = 0;
		}
		if (bFuc)
			SetCursor(wxCursor(_T("./skin/HandOpen.cur"), wxBITMAP_TYPE_CUR));
		else
		{
			m_stMP.emFuc = IMGPL_CMD::NONE;
			SetCursor(wxCursor(wxCURSOR_ARROW));
		}
	}
	break;
	case IMGPL_CMD::IMG_ZRECT:		// zoom rect
	{
		if (m_stMP.iState != 0)
		{
			m_stMP.iState = 0;
			ReleaseMouse();
			Refresh(false);
		}
		if (!bFuc)
		{
			m_stMP.emFuc = IMGPL_CMD::NONE;
			SetCursor(wxCursor(wxCURSOR_ARROW));
		}
	}
	break;
	case IMGPL_CMD::SEL_FAINT:
	{
		if (m_stMP.iState != 0)
		{
			m_stMP.iState = 0;
			ReleaseMouse();
			Refresh(false);
		}
		if (!bFuc)
		{
			m_stMP.emFuc = IMGPL_CMD::NONE;
			SetCursor(wxCursor(wxCURSOR_ARROW));
		}
	}
	break;
	case IMGPL_CMD::SEL_MIN:
	{
		if (m_stMP.iState != 0)
		{
			m_stMP.iState = 0;
			ReleaseMouse();
			Refresh(false);
		}
		if (!bFuc)
		{
			m_stMP.emFuc = IMGPL_CMD::NONE;
			SetCursor(wxCursor(wxCURSOR_ARROW));
		}
	}
	break;
	case IMGPL_CMD::SEL_MAX:
	{
		if (m_stMP.iState != 0)
		{
			m_stMP.iState = 0;
			ReleaseMouse();
			Refresh(false);
		}
		if (!bFuc)
		{
			m_stMP.emFuc = IMGPL_CMD::NONE;
			SetCursor(wxCursor(wxCURSOR_ARROW));
		}
	}
	break;
	default:
		break;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
/**< paint wnd. */
void ImagePanel::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxSize szWnd = GetClientSize();

	// erase background
	if (!m_dcMem.IsOk())
		return;
	m_dcMem.Clear();

	//--------------------Begin double buffer draw

	// draw image
	if (m_img.IsOk())
	{
		wxMemoryDC dcImg;
		wxBitmap bmp(m_img);
		dcImg.SelectObject(bmp);
		m_dcMem.StretchBlit( (wxCoord)m_rcDest.m_x, (wxCoord)m_rcDest.m_y,
		                     (wxCoord)m_rcDest.m_width, (wxCoord)m_rcDest.m_height,
		                     (wxDC*)&dcImg,
		                     (wxCoord)m_rcSrc.m_x, (wxCoord)m_rcSrc.m_y,
		                     (wxCoord)m_rcSrc.m_width, (wxCoord)m_rcSrc.m_height );
	}

	// draw sel rect fot zoom
	if (m_stMP.emFuc == IMGPL_CMD::IMG_ZRECT && m_stMP.iState == 2)
	{
		wxPen pen(wxColour(0x00ff0000), 1, wxPENSTYLE_DOT);
		m_dcMem.SetPen(pen);
		m_dcMem.SetLogicalFunction(wxOR_REVERSE);
		m_dcMem.SetBrush(wxNullBrush);
		m_dcMem.DrawRectangle(m_stMP.rcSel);
		m_dcMem.SetLogicalFunction(wxCOPY);
		m_dcMem.SetPen(wxNullPen);
	}
	// draw sel circle
	else if ((m_stMP.emFuc == IMGPL_CMD::SEL_FAINT || m_stMP.emFuc == IMGPL_CMD::SEL_MIN || m_stMP.emFuc == IMGPL_CMD::SEL_MAX)
	         && m_stMP.iState == 2
	         && m_stMP.rcSel.width > 0)
	{
		wxPen pen(wxColour(0x00ff0000), 1, wxPENSTYLE_DOT);
		m_dcMem.SetPen(pen);
		m_dcMem.SetLogicalFunction(wxOR_REVERSE);
		m_dcMem.SetBrush(wxNullBrush);
		m_dcMem.DrawCircle(m_stMP.rcSel.x, m_stMP.rcSel.y, m_stMP.rcSel.width);
		m_dcMem.SetLogicalFunction(wxCOPY);
		m_dcMem.SetPen(wxNullPen);
	}

	//--------------------End double buffer draw
	dc.Blit(0, 0, szWnd.x, szWnd.y, &m_dcMem, 0, 0);
}

/**< erase background, do nothing to shield auto erase */
void ImagePanel::OnErase(wxEraseEvent& event)
{
	// skip auto erase, so do nothing
	//event.Skip(true);
}

/**< on wnd resized */
void ImagePanel::OnSize(wxSizeEvent& event)
{
	if (!m_img.IsOk())
		return;
	// ensure the new size is valid
	if (event.m_size.x < 2 || event.m_size.y < 2)
		return;

	// the center move of the wnd
	wxPoint2DDouble szMove;
	szMove.m_x = (event.m_size.x - m_szClient.x)/2.0;
	szMove.m_y = (event.m_size.y - m_szClient.y)/2.0;

	wxRect2DDouble rcImg(0.0, 0.0, 0.0, 0.0);		// img rect
	wxRect2DDouble rcWnd(0.0, 0.0, event.m_size.x, event.m_size.y);	// new wnd rect
	do
	{
		wxSize szImg = m_img.GetSize();
		rcImg.m_width = szImg.GetWidth();
		rcImg.m_height = szImg.GetHeight();
	}
	while(false);

	// current image rect in wnd coordinate
	wxRect2DDouble rcNew;
	rcNew.m_x = m_rcDest.m_x - m_rcSrc.m_x*m_dScale;
	rcNew.m_y = m_rcDest.m_y - m_rcSrc.m_y*m_dScale;
	rcNew.m_width = rcImg.m_width*m_dScale;
	rcNew.m_height = rcImg.m_height*m_dScale;
	// then horizontal move the rect
	if ((rcNew.m_x + szMove.m_x) > rcWnd.m_width/2.0)
		szMove.m_x = (int)(rcWnd.m_width/2.0 - rcNew.m_x);
	else if ((rcNew.GetRight() + szMove.m_x) <= rcWnd.m_width/2.0)
		szMove.m_x = (int)(rcWnd.m_width/2.0 - rcNew.GetRight());
	// and then vertical move the rect
	if ((rcNew.m_y + szMove.m_y) > rcWnd.m_height/2.0)
		szMove.m_y = (int)(rcWnd.m_height/2.0 - rcNew.m_y);
	else if ((rcNew.GetBottom() + szMove.m_y) <= rcWnd.m_height/2.0)
		szMove.m_y = (int)(rcWnd.m_height/2.0 - rcNew.GetBottom());
	rcNew.Offset(szMove);

	// the dest rect is the intersection of new rect  and wnd rect
	wxRect2DDouble::Intersect(rcNew, rcWnd, &m_rcDest);

	// calculate the src rect
	m_rcSrc.m_x = (m_rcDest.m_x - rcNew.m_x)/m_dScale;
	m_rcSrc.m_y = (m_rcDest.m_y - rcNew.m_y)/m_dScale;
	m_rcSrc.m_width = m_rcDest.m_width/m_dScale;
	m_rcSrc.m_height = m_rcDest.m_height/m_dScale;
	// ensure the src rect is valid
	if (m_rcSrc.m_x < 0.0)
		m_rcSrc.m_x = 0.0;
	if (m_rcSrc.m_y < 0.0)
		m_rcSrc.m_y = 0.0;
	if (m_rcSrc.GetRight() > rcImg.m_width)
		m_rcSrc.m_width = rcImg.m_width - m_rcSrc.m_x;
	if (m_rcSrc.GetBottom() > rcImg.m_height)
		m_rcSrc.m_height = rcImg.m_height - m_rcSrc.m_y;

	// log the size
	m_szClient = event.m_size;

	// update panel
	//Refresh(false);
}

/**< context menu */
void ImagePanel::OnContextMenu(wxContextMenuEvent& event)
{
	wxMenu* pMenu = new wxMenu();
	wxASSERT_MSG(pMenu != nullptr, _T("Create Popup Menu failed."));
	wxMenuItem* pMenuItem = nullptr;
	// group 1
	if (m_img.IsOk())
	{
		pMenuItem = new wxMenuItem(pMenu, ID_CMENU_SAVE, _("&Save Image"), _("Save the Image"));
		pMenu->Append(pMenuItem);
	}
	// popup
	if (pMenuItem != nullptr)
	{
		wxPoint pt = event.GetPosition();
		if (pt == wxDefaultPosition)
		{
			// position invalide, get the mouse position
			pt = wxGetMousePosition();
			wxRect rc = GetScreenRect();
			if (!rc.Contains(pt))
			{
				// mouse is't in the panel, get the panel center
				pt.x = rc.x + rc.width/2;
				pt.y = rc.y + rc.height/2;
			}
		}
		pt = ScreenToClient(pt);
		PopupMenu(pMenu, pt);
	}
	delete pMenu;
}

/**< kill focus. if capture mouse, should release */
void ImagePanel::OnKillFocus(wxFocusEvent& event)
{
	EndDrag(false);
}

/**< invoke when mouse left button up */
void ImagePanel::OnMouseLD(wxMouseEvent& event)
{
	if (!m_img.IsOk())
		return;
	// the point should be on the image
	if (event.m_x < m_rcDest.m_x || event.m_x >= m_rcDest.GetRight()
	        || event.m_y < m_rcDest.m_y || event.m_y >= m_rcDest.GetBottom())
		return;

	switch (m_stMP.emFuc)
	{
	case IMGPL_CMD::IMG_MOVE:     // image move
		MLDImgMove(event);
		break;
	case IMGPL_CMD::IMG_ZRECT:     // zoom rect
		MLDImgZRect(event);
		break;
	case IMGPL_CMD::SEL_FAINT:
		MLDSelFaint(event);
		break;
	case IMGPL_CMD::SEL_MIN:
		MLDSelMin(event);
		break;
	case IMGPL_CMD::SEL_MAX:
		MLDSelMax(event);
		break;
	default:
		break;
	}
}

/**< invoke when mouse left button up */
void ImagePanel::OnMouseLU(wxMouseEvent& event)
{
	if (!m_img.IsOk())
		return;
	if (m_stMP.iState == 0)
		return;
	switch (m_stMP.emFuc)
	{
	case IMGPL_CMD::IMG_MOVE:     // image move
		MLUImgMove(event);
		break;
	case IMGPL_CMD::IMG_ZRECT:     // zoom rect
		MLUImgZRect(event);
		break;
	case IMGPL_CMD::SEL_FAINT:
		MLUSelFaint(event);
		break;
	case IMGPL_CMD::SEL_MIN:
		MLUSelMin(event);
		break;
	case IMGPL_CMD::SEL_MAX:
		MLUSelMax(event);
		break;
	default:
		break;
	}
}

/**< invoke when mouse move */
void ImagePanel::OnMouseMove(wxMouseEvent& event)
{
	if (!m_img.IsOk())
		return;
	if (m_stMP.iState == 0)
		return;
	switch (m_stMP.emFuc)
	{
	case IMGPL_CMD::IMG_MOVE:
		MMVImgMove(event);
		break;
	case IMGPL_CMD::IMG_ZRECT:
		MMVImgZRect(event);
		break;
	case IMGPL_CMD::SEL_FAINT:
		MMVSelFaint(event);
		break;
	case IMGPL_CMD::SEL_MIN:
		MMVSelMin(event);
		break;
	case IMGPL_CMD::SEL_MAX:
		MMVSelMax(event);
		break;
	default:
		break;
	}
}

/**< ContextMenu save image */
void ImagePanel::OnCmenuSave(wxCommandEvent& event)
{
	static const wxString EXTS[] =
	{
		_T("bmp"),
		_T("jpg"),
		_T("png"),
		_T("tif")
	};
	// choose images file's name
	wxFileDialog dlgFile(this, _("Save Image"),
	                     _T(""), _T(""),
	                     _T("Bitmap|*.bmp|JPEG|*.jpg;*.jpeg|PNG|*.png|TIFF|*.tif;*.tiff"),
	                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	dlgFile.SetFilterIndex(2);	// png
	if(dlgFile.ShowModal() != wxID_OK)
		return;
	// extention
	int iExt = dlgFile.GetFilterIndex();
	// path modify
	wxFileName fln(dlgFile.GetPath());
	fln.ClearExt();
	fln.SetExt(EXTS[iExt]);
	wxString strFile = fln.GetFullPath();
	m_img.SaveFile(strFile);
}
