/**
 *
 * \file SpotDtDlg.h
 * \author Author:ZHD
 * \date Date:26/03/2014
 *
 * Used to detect the gel spot in electrophoresis image
 *
 */
#pragma once
#ifndef SPOTDTDLG_H
#define SPOTDTDLG_H

#include <wx/dialog.h>
#include "GEIMDef.h"
#include "ImagePanel.h"

class SpotDtDlg : public wxDialog
{
	public:
        /** \brief  Construct
         *
         * simple construct, after this, invoke Create function to create dialog
		 *
         */
		SpotDtDlg();
        /** \brief Construct and Create
         *
         * \param wxWindow* parent		[IN] this dialog's parent wnd
         * \param const wxString& title	[IN] dialog's title
         * \param const wxPoint& pos	[IN] dialog's init position
         * \param const wxSize& sz		[IN] dialog's init size
         * \param long style						[IN] dialog's style
         * \param const wxString& name [IN] reserve
         *
         */
		SpotDtDlg(wxWindow* parent,
					wxWindowID id = wxID_ANY,
					const wxString& title = _("Spot Detection"),
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& sz = wxDefaultSize,
					long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER,
					const wxString& name = wxDialogNameStr);
        /** \brief destruct
         *
         */
		virtual ~SpotDtDlg();

        /** \brief create dialog
         *
         * \param wxWindow* parent		[IN] this dialog's parent wnd
         * \param const wxString& title	[IN] dialog's title
         * \param const wxPoint& pos	[IN] dialog's init position
         * \param const wxSize& sz		[IN] dialog's init size
         * \param long style						[IN] dialog's style
         * \param const wxString& name [IN] reserve
         * \return bool true:success false:failed
         */
		bool Create(wxWindow* parent,
					wxWindowID id = wxID_ANY,
					const wxString& title = _("Spot Detection"),
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& sz = wxDefaultSize,
					long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER,
					const wxString& name = wxDialogNameStr);

        /** \brief Initialize member, include gel images
         *
         * \param const wxArrayPtrVoid* pAryImgs [IN] the pointer of array fill images' pointer
         * \return bool true:success false:failed
         *
         */
		bool Init(const wxArrayPtrVoid* pAryImgs);

        /** \brief obtain the detection parameter
         *
         * \return ST_DTPARAM&	the parameter reference
         *
         */
		inline ST_DTPARAM& DtParam()
		{
			return m_stDtParam;
		}

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
        /** \brief obtain detect param from ui control
         *
         * \return bool	true:success false:failed
         *
         */
        bool GetParam();
        /** \brief obtain the faint spot in special circle
         *
         * \param rc wxRect&	[IN] -x:circle center -y:circle center -width:radius
         * \return bool	true:success false:failed
         *
         */
        bool GetFaintSpot(const wxRect& rc);

        // image operate buttons
        void OnZoomIN(wxCommandEvent& event);
        void OnZoomOut(wxCommandEvent& event);
        void OnZoomRect(wxCommandEvent& event);
        void OnZoomFit(wxCommandEvent& event);
        void OnZoomActual(wxCommandEvent& event);
        void OnImgMove(wxCommandEvent& event);
        void OnDtFaint(wxCommandEvent& event);
        void OnDtMin(wxCommandEvent& event);
        void OnDtMax(wxCommandEvent& event);
        void OnBtnsUpdate(wxUpdateUIEvent& event);
        void OnImgplNtfy(wxImgplEvent& event);
        // other controls
        void OnRbxMedianUpdate(wxUpdateUIEvent& event);
        void OnRbxGaussUpdate(wxUpdateUIEvent& event);
        void OnSpMaxUpdate(wxUpdateUIEvent& event);
        void OnCiImage(wxCommandEvent& event);
        void OnBtnTestParam(wxCommandEvent& event);
        void OnBtnOK(wxCommandEvent& event);
        void OnBtnProcUpdate(wxUpdateUIEvent& event);

		/**< control id */
        // tools buttons
        static const long ID_BMPBTN_IMG_ZOOMIN;
        static const long ID_BMPBTN_IMG_ZOOMOUT;
        static const long ID_BMPBTN_IMG_ZOOMRECT;
        static const long ID_BMPBTN_IMG_ZOOMFIT;
        static const long ID_BMPBTN_IMG_ZOOMACTUAL;
        static const long ID_BMPBTN_IMG_MOVE;
        // image selector
        static const long ID_CI_IMAGE;
        // detect param btn
        static const long ID_BMPBTN_DT_FAINT;
        static const long ID_BMPBTN_DT_MIN;
        static const long ID_BMPBTN_DT_MAX;
        static const long ID_CB_BKGCORRECT;
        // filter param
        static const long ID_CB_MEDIAN;
        static const long ID_CB_GAUSS;
        static const long ID_RBX_MEDIAN;
        static const long ID_RBX_GAUSS;
        // detect param edit
        static const long ID_ED_FAINT_X;
        static const long ID_ED_FAINT_Y;
        static const long ID_SPD_FAINT_T;
        static const long ID_SP_MIN;
        static const long ID_SP_MAX;
        static const long ID_SPD_ASPECT;
        static const long ID_ED_SPOTNUM;

		/**< detect param */
		ST_DTPARAM m_stDtParam;

		/**< image panel to dispaly iamge */
		ImagePanel* m_pImgPanel = nullptr;

		/**< images's pointer arrary for batch peocess */
		const wxArrayPtrVoid* m_pAryImgs = nullptr;

		/**< iamge to display */
		wxImage m_imgDisp;

		DECLARE_CLASS(SpotDtDlg)
		DECLARE_EVENT_TABLE()
};

#endif // SPOTDTDLG_H
