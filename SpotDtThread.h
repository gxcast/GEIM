#pragma once
#ifndef SPOTDTTHREAD_H
#define SPOTDTTHREAD_H

// generic define
#include "GEIMDef.h"
// event handler
#include <wx/event.h>
// thread include
#include <wx/thread.h>


class SpotDtThread : public wxThread
{
	public:
		// 通知事件ID
		static const long ID;

        /** \brief construct
         *
         * \param host wxEvtHandler*		[IN] host, handle the event
         * \param param ST_DTPARAM&			[IN] detect param
         * \param imgs wxArrayPtrVoid&		[IN] origin images
         * \param disps wxArrayPtrVoid&		[IN] diplay images
         * \param sets LS_DTRESULT&			[INOUT] the detect result
         *
         */
		SpotDtThread(wxEvtHandler* host, ST_DTPARAM& param,
					wxArrayPtrVoid& imgs, wxArrayPtrVoid& disps, LS_DTRESULT& sets);
		virtual ~SpotDtThread();

		// thread's entry
		virtual void* Entry();

        /** \brief destroy the detect result list
         *
         * \param pLs LS_DTRESULT*      [IN] the detect result list
         * \return bool true:success false:failed
         *
         */
        static bool DestroyDtResult(LS_DTRESULT* pLs);

	protected:

	private:
        /** \brief perform the detection algorithm, at the same, display the result
         *
         * \param stDtParam ST_DTPARAM& [IN] the detection parameter
         * \return bool true:success false:failed
         *
         */
        bool SpotDtBatch(ST_DTPARAM& stDtParam);
        /** \brief display the detect result
         *
         * \param pImg wxImage*         [IN] origin image
         * \param ret ST_DTRESULT&      [IN] the detect reuslt
         * \return bool true:success false:failed
         *
         */
        bool DispDtResult(wxImage* pImg, ST_DTRESULT& ret);


        /**< host */
        wxEvtHandler* m_pHost = nullptr;
        /**< the detection parameter */
        ST_DTPARAM m_stDtParam;
        /**< array contain origin images' pointer */
        wxArrayPtrVoid& m_aryImgs;
        /**< array contain origin images' pointer for dispaly */
        wxArrayPtrVoid& m_aryImgsDisp;
        /**< all the image's detection result */
        LS_DTRESULT& m_lsDtResult;
};

#endif // SPOTDTTHREAD_H
