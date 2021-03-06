#pragma once
#ifndef SPOTMTTHREAD_H
#define SPOTMTTHREAD_H

// generic define
#include "GEIMDef.h"
// event handler
#include <wx/event.h>
// thread include
#include <wx/thread.h>


class SpotMtThread : public wxThread
{
public:
	// 通知事件ID
	static const long ID;

	/** \brief construct
	 *
	 * \param host wxEvtHandler*	[IN] host, handle the event
	 * \param dt LS_DTRESULT&		[IN] the detect result
	 * \param imgs wxArrayPtrVoid&	[IN] origin images
	 * \param disps wxArrayPtrVoid&	[IN] diplay images
	 * \param mt ST_MTRESULT&		[OUT] the match result
	 *
	 */
	SpotMtThread(wxEvtHandler* host, LS_DTRESULT& dt,
	             wxArrayPtrVoid& imgs, wxArrayPtrVoid& disps, ST_MTRESULT& mt);
	virtual ~SpotMtThread();

	// thread's entry
	virtual void* Entry();

	/** \brief detroy the mt param
	 *
	 * \param pstParm ST_MTPARAM*   [IN] param to destroy
	 * \return bool true:success false:failed
	 *
	 */
	static bool DestroyMTParam(ST_MTPARAM* pstParm);
	/** \brief destroy the match result
	 *
	 * \param pRst ST_MTRESULT*	[IN] the match result's pointer
	 * \return bool true:success false:failed
	 *
	 */
	static bool DestroyMtResult(ST_MTRESULT* pRst);

protected:

private:
	/** \brief match process
	 *
	 * \return bool true:success false:failed
	 *
	 */
	bool SpotMatch();
	/** \brief init match parametar of a image
	 *
	 * \param stParam ST_MTPARAM&	[IN] detect results
	 * \param id int				[IN] which image will be use
	 * \return bool true:success false:failed
	 *
	 */
	bool InitParam(ST_MTPARAM& stParam, int id);

	/** \brief display spots' character in each image
	 *
	 * \param paramA ST_MTPARAM&	[IN] match info of image A
	 * \param paramB ST_MTPARAM&	[IN] match info of image B
	 * \return bool	true
	 *
	 */
	bool DispMtResult_crt(ST_MTPARAM& paramA, ST_MTPARAM& paramB);
	/** \brief draw piar in each image, using the same color
	 *
	 * \param stParam ST_MTPARAM&	[IN] match infomation
	 * \param id int				[IN] which iamge
	 * \param bgi int				[IN] if draw back-image, bit-domain 0x01:image A, 0x02 :image B
	 * \return bool true:success false:failed
	 *
	 */
	bool DispMtResult_icp(ST_MTPARAM& paramA, ST_MTPARAM& paramB, int bgi = 0);
    /** \brief union two image into one, and draw pair-line
     *
	 * \param paramA ST_MTPARAM&	[IN] match info of image A
	 * \param paramB ST_MTPARAM&	[IN] match info of image B
     * \return bool true, false
     *
     */
	bool DispMtResult_vec(ST_MTPARAM& paramA, ST_MTPARAM& paramB);

	/**< host */
	wxEvtHandler* m_pHost = nullptr;
	/**< all the image's detection result */
	LS_DTRESULT& m_lsDtResult;
	/**< array contain origin images' pointer */
	wxArrayPtrVoid& m_aryImgs;
	/**< array contain origin images' pointer for dispaly */
	wxArrayPtrVoid& m_aryImgsDisp;
	/**< the match result */
	ST_MTRESULT& m_stMtResult;
};

#endif // SPOTMTTHREAD_H
