#include "HttpManager.h"
#include <logger.h>
#include <chrono>
FHttpManager::FHttpManager():DeferredDestroyDelay(10)
{
}

FHttpManager::~FHttpManager()
{
	if (Thread)
	{
		Thread->StopThread();
		delete Thread;
		Thread = nullptr;
	}
}

void FHttpManager::Initialize()
{
	Thread = CreateHttpThread();
	Thread->StartThread();
}

void FHttpManager::AddRequest(const std::shared_ptr<IHttpRequest>& Request)
{
	std::scoped_lock(RequestLock);
	Requests.push_back(Request);
}

void FHttpManager::RemoveRequest(const std::shared_ptr<IHttpRequest>& Request)
{
	std::scoped_lock(RequestLock);
	// Keep track of requests that have been removed to be destroyed later
	PendingDestroyRequests.push_back(FRequestPendingDestroy(DeferredDestroyDelay, Request));
	std::erase( Requests,Request);
}

bool FHttpManager::IsValidRequest(const IHttpRequest* RequestPtr) const
{
	bool bResult = false;
	for (const auto& Request : Requests)
	{
		if (Request.get() == RequestPtr)
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

void FHttpManager::Flush(bool bShutdown)
{
	if (bShutdown)
	{
		if (Requests.size())
		{
			LOG_INFO("Http module shutting down, but needs to wait on {} outstanding Http requests:", Requests.size());
		}
		// Clear delegates since they may point to deleted instances
		for (auto& Request : Requests)
		{
			
			Request->OnProcessRequestComplete() = nullptr;
			Request->OnRequestProgress() = nullptr;
			LOG_INFO(("	verb={} url={} status={}"), Request->GetVerb(), Request->GetURL(), EHttpRequestStatus::ToString(Request->GetStatus()));
		}
	}

	// block until all active requests have completed
	auto LastTime = std::chrono::steady_clock::now();
	while (Requests.size() > 0)
	{
		auto AppTime = std::chrono::steady_clock::now();
		Tick(std::chrono::duration_cast<std::chrono::seconds>(AppTime - LastTime).count());
		LastTime = AppTime;
		if (Requests.size() > 0)
		{
			Thread->Tick();
		}
	}
}

bool FHttpManager::Tick(float DeltaSeconds)
{
	std::scoped_lock(RequestLock);

	// Tick each active request
	for (auto& Request: Requests)
	{
		Request->Tick(DeltaSeconds);
	}
	// Tick any pending destroy objects
	for (auto itr= PendingDestroyRequests.begin();itr!= PendingDestroyRequests.end();)
	{
		FRequestPendingDestroy& Request = *itr;
		Request.TimeLeft -= DeltaSeconds;
		if (Request.TimeLeft <= 0)
		{
			PendingDestroyRequests.erase(itr);
		}
		else {
			itr++;
		}
	}

	std::vector<IHttpThreadedRequest*> CompletedThreadedRequests;
	Thread->GetCompletedRequests(CompletedThreadedRequests);

	// Finish and remove any completed requests
	for (IHttpThreadedRequest* CompletedRequest : CompletedThreadedRequests)
	{
		auto itr=std::find(Requests.begin(), Requests.end(), CompletedRequest);
		if (itr == Requests.end()) {
			continue;
		}
		// Keep track of requests that have been removed to be destroyed later
		PendingDestroyRequests.push_back(FRequestPendingDestroy(DeferredDestroyDelay, *itr));
		Requests.erase(itr);
		CompletedRequest->FinishRequest();
	}
	// keep ticking
	return true;
}

void FHttpManager::AddThreadedRequest(const std::shared_ptr<IHttpThreadedRequest>& Request)
{
	AddRequest(Request);
	Thread->AddRequest(Request.get());
}

void FHttpManager::CancelThreadedRequest(const std::shared_ptr<IHttpThreadedRequest>& Request)
{
	Thread->CancelRequest(Request.get());
}

void FHttpManager::DumpRequests() const
{
	std::scoped_lock(RequestLock);
	LOG_INFO("------- ({}) Http Requests", Requests.size());
	for (const auto& Request : Requests)
	{
		LOG_INFO("verb=[{}] url=[{}] status={}", Request->GetVerb(),Request->GetURL(), EHttpRequestStatus::ToString(Request->GetStatus()));
	}
}

FHttpThread* FHttpManager::CreateHttpThread()
{
	return new FHttpThread();
}
