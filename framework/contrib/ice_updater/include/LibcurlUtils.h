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
#ifndef LIBCURLUTILS_H
#define LIBCURLUTILS_H

#include <iostream>
#include <string>
#include "INetworkingTool.h"

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_CURL
#  include <curl/curl.h>
#  include <curl/curlver.h>

// A macro to help determine if the version of curl is new enough
#define CURL_VERSION_LESS_THAN(major,minor,patch)                       \
  ((LIBCURL_VERSION_MAJOR < (major) ||                                  \
    (LIBCURL_VERSION_MAJOR == (major) && (LIBCURL_VERSION_MINOR < (minor) || \
                                          (LIBCURL_VERSION_MINOR == (minor) && \
                                           LIBCURL_VERSION_PATCH < (patch))))) ? 1 : 0)

#endif

/**
 * LibcurlUtils is a utility class used to transmit and receive information
 * with the POST and GET HTTP methods using the C library libcurl.
 */
class LibcurlUtils : public INetworkingTool
{

private:

#ifdef LIBMESH_HAVE_CURL
  /**
   * A handle to the cURL object.
   */
  CURL * curl;

  /**
   * The result of the last curl call.
   */
  CURLcode result;

  /**
   * A char array an error from the last curl call.
   */
  char error[CURL_ERROR_SIZE];
#endif

  /**
   * A flag indicating whether or not cURL calls will skip peer
   * certificate verification for HTTPS urls. This flag should
   * only be set to true for testing purposes.
   */
  bool ignoreSslPeerVerification;

  /**
   * True by default.  When true, set (CURLOPT_NOPROXY, "*") when
   * calling libcurl APIs.  Call the setNoProxyFlag() setter to change
   * the value.
   */
  bool noProxyFlag;

public:

  /**
   * The Constructor.
   */
  LibcurlUtils();

  /**
   * The Destructor.
   */
  ~LibcurlUtils();

  /**
   * Uses libcurl and GET to return the contents located at url.
   *
   * @param url The URL of the GET request.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @return The contents at the URL or an error message if one took place.
   */
  std::string get(std::string url, std::string username, std::string password);

  /**
   * Uses libcurl and POST to transmit value at url.
   *
   * @param url The url that is used to post the value.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @param value The value that is posted to the url.
   * @return A std::string containing the error if one took place. Else returns an empty std::string.
   */
  std::string post(std::string url, std::string value, std::string username, std::string password);

  /**
   * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
   * set to true then cURL will skip peer certificate verification for HTTPS urls.
   * This flag should only be set to true for testing purposes.
   *
   * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
   */
  virtual void setIgnoreSslPeerVerification(bool ignoreSslPeerVerification);

  /**
   * Sets the noProxyFlag's value to 'val'.
   *
   * @param val The new value for the noProxyFlag.
   */
  virtual void setNoProxyFlag(bool val) { noProxyFlag = val; }

  /**
   * A callback required by the C libcurl library to write the contents returned by get() to a buffer.
   *
   * @param data The get data.
   * @param size The size of each item.
   * @param nmemb The number of items in memory.
   * @param buffer The buffer to store the get() contents.
   * @return The amount written which should be size * nmemb.
   */
  static int writeGetData(char * data, size_t size, size_t nmemb, std::string buffer);
};

#endif
