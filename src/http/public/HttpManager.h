#pragma once
#include "IHttpRequest.h"
#include "IHttpResponse.h"
#include "HttpThread.h"
#include <list>
class FHttpManager
{
public:

	// FHttpManager

	/**
	 * Constructor
	 */
	FHttpManager();

	/**
	 * Destructor
	 */
	~FHttpManager();

	/**
	 * Initialize
	 */
	void Initialize();

	/**
	 * Adds an Http request instance to the manager for tracking/ticking
	 * Manager should always have a list of requests currently being processed
	 *
	 * @param Request - the request object to add
	 */
	void AddRequest(const std::shared_ptr<IHttpRequest>& Request);

	/**
	 * Removes an Http request instance from the manager
	 * Presumably it is done being processed
	 *
	 * @param Request - the request object to remove
	 */
	void RemoveRequest(const std::shared_ptr<IHttpRequest>& Request);

	/**
	* Find an Http request in the lists of current valid requests
	*
	* @param RequestPtr - ptr to the http request object to find
	*
	* @return true if the request is being tracked, false if not
	*/
	bool IsValidRequest(const IHttpRequest* RequestPtr) const;

	/**
	 * Block until all pending requests are finished processing
	 *
	 * @param bShutdown true if final flush during shutdown
	 */
	void Flush(bool bShutdown);

	/**
	 * FTicker callback
	 *
	 * @param DeltaSeconds - time in seconds since the last tick
	 *
	 * @return false if no longer needs ticking
	 */
	virtual bool Tick(float DeltaSeconds);

	/**
	 * Add a http request to be executed on the http thread
	 *
	 * @param Request - the request object to add
	 */
	void AddThreadedRequest(const std::shared_ptr<IHttpThreadedRequest>& Request);

	/**
	 * Mark a threaded http request as cancelled to be removed from the http thread
	 *
	 * @param Request - the request object to cancel
	 */
	void CancelThreadedRequest(const std::shared_ptr<IHttpThreadedRequest>& Request);

	/**
	 * List all of the Http requests currently being processed
	 *
	 * @param Ar - output device to log with
	 */
	void DumpRequests() const;

protected:
	/**
	 * Create HTTP thread object
	 *
	 * @return the HTTP thread object
	 */
	virtual FHttpThread* CreateHttpThread();


protected:
	/** List of Http requests that are actively being processed */
	std::list<std::shared_ptr<IHttpRequest>> Requests;
	std::mutex RequestLock;
	/** Keep track of a request that should be deleted later */
	class FRequestPendingDestroy
	{
	public:
		FRequestPendingDestroy(float InTimeLeft, const std::shared_ptr<IHttpRequest>& InHttpRequest)
			: TimeLeft(InTimeLeft)
			, HttpRequest(InHttpRequest)
		{}

		inline bool operator==(const FRequestPendingDestroy& Other) const
		{
			return Other.HttpRequest == HttpRequest;
		}

		float TimeLeft;
		std::shared_ptr<IHttpRequest> HttpRequest;
	};

	/** Dead requests that need to be destroyed */
	std::list<FRequestPendingDestroy> PendingDestroyRequests;

	FHttpThread* Thread;
	float DeferredDestroyDelay;
};
