/*******************************************************************************
 * Copyright (c) 2015, UT-Battelle, LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in
 *  the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *  contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*******************************************************************************/
#include "LibcurlUtils.h"
#include <iostream>
#include <sstream>
#include "Moose.h"
#include "MooseError.h"

/**
 * The Constructor.
 */
LibcurlUtils::LibcurlUtils() :
    ignoreSslPeerVerification(false),
    noProxyFlag(true)
{

#ifdef LIBMESH_HAVE_CURL
  // Create the curl handle
  curl = curl_easy_init();
#else
  mooseError("You tried to use the LibcurlUtils class, but CURL is not available.");
#endif

  // Set the ignoreSslPeerVerification flag to false.
}

/**
 * The Destructor.
 */
LibcurlUtils::~LibcurlUtils()
{
#ifdef LIBMESH_HAVE_CURL
  // Cleanup handle
  curl_easy_cleanup(curl);
#endif
}

/**
 * Uses libcurl and GET to return the contents located at url.
 *
 * @param url The URL of the GET request.
 * @return The contents at the URL or an error message if one took place.
 */
std::string LibcurlUtils::get(std::string url, std::string username, std::string password)
{
#ifdef LIBMESH_HAVE_CURL
  // If handle was successfully created
  if (curl)
  {
    // String to hold contents retrieved with GET.
    std::string buffer;

    // Reinitialize curl handle
    curl_easy_reset(curl);

    // Set the error buffer
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, url.data());

    // Set the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeGetData);

#if !CURL_VERSION_LESS_THAN(7,9,7)
    // Set the data pointer to write to the provided buffer
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
#endif

    // If the ignoreSslPeerVerification flag is set to true,
    // do not verify server certificate
    if (ignoreSslPeerVerification)
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

#if !CURL_VERSION_LESS_THAN(7,19,4)
    // Set the noproxy command line option if the user requested it.
    if (noProxyFlag)
      curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
#endif

#if !CURL_VERSION_LESS_THAN(7,19,1)
    // Set the username and password
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.data());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.data());
#endif

    // Attempt to retrieve from the remote page
    result = curl_easy_perform(curl);

    // Check the result
    if (result == CURLE_OK)
    {
      // Create a variable to hold HTTP response code
      long http_code;

      // Call easy info function to get the code of the last call.
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      // If the response code is other than 200 OK
      if (http_code != 200)
      {
        // Create a string to describe the error
        std::string error = "A GET request to ";
        std::stringstream ss;
        ss << http_code;
        error += url;
        error += " returned HTTP code ";
        error += ss.str();
        error += ".";

        // Return the error.
        return error;
      }

      // Return the contents of the GET response
      return buffer;
    }
    else
      return std::string(error);

  }
  else
    return std::string("CURL could not be initialized.");

#else
  libmesh_ignore(url);
  libmesh_ignore(username);
  libmesh_ignore(password);
  mooseError("You tried to use the LibcurlUtils class, but CURL is not available.");
  return std::string("");
#endif
}

/**
 * Uses libcurl and POST to transmit value at url.
 *
 * @param url The url that is used to post the value.
 * @param value The value that is posted to the url.
 * @return A string containing the error if one took place. Else returns an empty string.
 */
std::string LibcurlUtils::post(std::string url, std::string value, std::string username, std::string password)
{
#ifdef LIBMESH_HAVE_CURL
  // If handle was successfully created and the value is not empty
  if (curl && !value.empty())
  {
    // Create string to post
    std::string data = "post=" + value;

    // Reinitialize curl handle
    curl_easy_reset(curl);

    // Set the POST field with data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data());

    // Set the error buffer
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

    // Set the URL for the post
    curl_easy_setopt(curl, CURLOPT_URL, url.data());

    // If the ignoreSslPeerVerification flag is set to true,
    // do not verify server certificate
    if (ignoreSslPeerVerification)
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    // Set the username and password
#if !CURL_VERSION_LESS_THAN(7,19,1)
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.data());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.data());
#endif

#if !CURL_VERSION_LESS_THAN(7,19,4)
    // Set the noproxy command line option if the user requested it.
    if (noProxyFlag)
      curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
#endif

    // Attempt to post to the remote page
    result = curl_easy_perform(curl);

    // Check the result
    if (result == CURLE_OK)
    {
      // Create a variable to hold HTTP response code
      long http_code;

      // Call easy info function to get the code of the last call.
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      // If the response code is other than 200 OK
      if (http_code != 200)
      {
        // Create a string to describe the error
        std::string error = "A POST request to ";
        std::stringstream ss;
        ss << http_code;
        error += url;
        error += " returned HTTP code ";
        error += ss.str();
        error += ".";

        // Return the error.
        return error;
      }

      // Return an empty string indicating no error
      return "";

    }
    else
      return std::string(error);
  }
  else
    return std::string("CURL could not be initialized.");

#else
  libmesh_ignore(url);
  libmesh_ignore(value);
  libmesh_ignore(username);
  libmesh_ignore(password);
  mooseError("You tried to use the LibcurlUtils class, but CURL is not available.");
  return std::string("");
#endif
}

/**
 * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
 * set to true then cURL will skip peer certificate verification for HTTPS urls.
 * This flag should only be set to true for testing purposes.
 *
 * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
 */
void LibcurlUtils::setIgnoreSslPeerVerification(bool ignoreSslPeerVerification)
{
  // Set the instance value to the value in ignoreSslPeerVerification.
  this->ignoreSslPeerVerification = ignoreSslPeerVerification;
}

/**
 * A callback required by the C libcurl library to write the contents returned by get() to a buffer.
 *
 * @param data The get data.
 * @param size The size of each item.
 * @param nmemb The number of items in memory.
 * @param buffer The buffer to store the get() contents.
 * @return The amount written which should be size * nmemb.
 */
int LibcurlUtils::writeGetData(char * data,  size_t size, size_t nmemb, std::string buffer)
{
  // Append the data to the buffer
  buffer.append(data, size * nmemb);

  // Compute the size written to buffer
  int result = size * nmemb;

  // Return the resulting size
  return result;
}
