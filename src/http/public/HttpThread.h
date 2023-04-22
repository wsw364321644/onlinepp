#pragma once
#include "IHttpRequest.h"
#include <thread>
#include <mutex>
class FHttpThread
{
public:

	FHttpThread();
	virtual ~FHttpThread();

	/**
	 * Start the HTTP thread.
	 */
	void StartThread();

	/**
	 * Stop the HTTP thread.  Blocks until thread has stopped.
	 */
	void StopThread();

	/**
	 * Add a request to begin processing on HTTP thread.
	 *
	 * @param Request the request to be processed on the HTTP thread
	 */
	void AddRequest(IHttpThreadedRequest* Request);

	/**
	 * Mark a request as cancelled.    Called on non-HTTP thread.
	 *
	 * @param Request the request to be processed on the HTTP thread
	 */
	void CancelRequest(IHttpThreadedRequest* Request);

	/**
	 * Get completed requests.  Clears internal arrays.  Called on non-HTTP thread.
	 *
	 * @param OutCompletedRequests array of requests that have been completed
	 */
	void GetCompletedRequests(std::vector<IHttpThreadedRequest*>& OutCompletedRequests);

	virtual void Tick();


protected:

	/**
	 * Tick on http thread
	 */
	virtual void HttpThreadTick(float DeltaSeconds);

	/**
	 * Start processing a request on the http thread
	 */
	virtual bool StartThreadedRequest(IHttpThreadedRequest* Request);

	/**
	 * Complete a request on the http thread
	 */
	virtual void CompleteThreadedRequest(IHttpThreadedRequest* Request);


protected:
	// Threading functions

	virtual bool Init() ;
	virtual uint32_t Run() ;
	virtual void Stop() ;
	virtual void Exit() ;


	void Process(std::vector<IHttpThreadedRequest*>& RequestsToCancel, std::vector<IHttpThreadedRequest*>& RequestsToStart, std::vector<IHttpThreadedRequest*>& RequestsToComplete);

	/** signal request to stop and exit thread */
	std::atomic_bool ExitRequest{false};

	/** Time in seconds to use as frame time when actively processing requests. 0 means no frame time. */
	double HttpThreadActiveFrameTimeInSeconds;
	/** Time in seconds to sleep minimally when actively processing requests. */
	double HttpThreadActiveMinimumSleepTimeInSeconds;
	/** Time in seconds to use as frame time when idle, waiting for requests. 0 means no frame time. */
	double HttpThreadIdleFrameTimeInSeconds;
	/** Time in seconds to sleep minimally when idle, waiting for requests. */
	double HttpThreadIdleMinimumSleepTimeInSeconds;
	/** Last time the thread has been processed. Used in the non-game thread. */
	double LastTime;

protected:
	/** Critical section to lock access to PendingThreadedRequests, CancelledThreadedRequests, and CompletedThreadedRequests */
	std::mutex RequestArraysLock;

	/**
	 * Threaded requests that are waiting to be processed on the http thread.
	 * Added to on non-HTTP thread, processed then cleared on HTTP thread.
	 */
	std::vector<IHttpThreadedRequest*> PendingThreadedRequests;

	/**
	 * Threaded requests that are waiting to be cancelled on the http thread.
	 * Added to on non-HTTP thread, processed then cleared on HTTP thread.
	 */
	std::vector<IHttpThreadedRequest*> CancelledThreadedRequests;

	/**
	 * Currently running threaded requests (not in any of the other arrays).
	 * Only accessed on the HTTP thread.
	 */
	std::vector<IHttpThreadedRequest*> RunningThreadedRequests;

	/**
	 * Threaded requests that have completed and are waiting for the game thread to process.
	 * Added to on HTTP thread, processed then cleared on game thread.
	 */
	std::vector<IHttpThreadedRequest*> CompletedThreadedRequests;

	/** Pointer to Runnable Thread */
	std::thread* Thread;
};
