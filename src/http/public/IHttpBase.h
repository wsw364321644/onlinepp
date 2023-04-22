#pragma once
#include <string>
#include <vector>
class IHttpBase
{
public:

	/**
	 * Get the URL used to send the request.
	 *
	 * @return the URL string.
	 */
	virtual std::string GetURL() = 0;

	/** 
	 * Gets an URL parameter.
	 * expected format is ?Key=Value&Key=Value...
	 * If that format is not used, this function will not work.
	 * 
	 * @param ParameterName - the parameter to request.
	 * @return the parameter value string.
	 */
	virtual std::string GetURLParameter(const std::string& ParameterName) = 0;

	/** 
	 * Gets the value of a header, or empty string if not found. 
	 * 
	 * @param HeaderName - name of the header to set.
	 */
	virtual std::string GetHeader(const std::string& HeaderName) = 0;

	/**
	 * Return all headers in an array in "Name: Value" format.
	 *
	 * @return the header array of strings
	 */
	virtual std::vector<std::string> GetAllHeaders() = 0;

	/**
	 * Shortcut to get the Content-Type header value (if available)
	 *
	 * @return the content type.
	 */
	virtual std::string GetContentType() = 0;

	/**
	 * Shortcut to get the Content-Length header value. Will not always return non-zero.
	 * If you want the real length of the payload, get the payload and check it's length.
	 *
	 * @return the content length (if available)
	 */
	virtual int32_t GetContentLength() = 0;

	/**
	 * Get the content payload of the request or response.
	 *
	 * @param Content - array that will be filled with the content.
	 */
	virtual const std::vector<uint8_t>& GetContent() = 0;

	/** 
	 * Destructor for overrides 
	 */
	virtual ~IHttpBase() {};
};

