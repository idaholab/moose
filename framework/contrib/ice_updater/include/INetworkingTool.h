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
#ifndef INETWORKINGTOOL_H_
#define INETWORKINGTOOL_H_

#include "MooseError.h"

/**
 *
 */
class INetworkingTool
{
public:

  virtual ~INetworkingTool()
  {
  }

  /**
   * Execute HTTP GET to return the contents located at url.
   *
   * @param url The URL of the GET request.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @return The contents at the URL or an error message if one took place.
   */
  virtual std::string get(std::string /*url*/,
                          std::string /*username*/,
                          std::string /*password*/)
  {
    mooseWarning("You are attempting to execute a GET but did not specify a valid ICE Updater INetworkingTool! No data will be received.");
    return std::string("");
  }

  /**
   * Execute HTTP POST to transmit value at url.
   *
   * @param url The url that is used to post the value.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @param value The value that is posted to the url.
   * @return A std::string containing the error if one took place. Else returns an empty std::string.
   */
  virtual std::string post(std::string /*url*/,
                           std::string /*value*/,
                           std::string /*username*/,
                           std::string /*password*/)
  {
    mooseWarning("You are attempting to execute a POST but did not specify a valid ICE Updater INetworkingTool! No data will be posted.");
    return std::string("");
  }

  /**
   * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
   * set to true then cURL will skip peer certificate verification for HTTPS urls.
   * This flag should only be set to true for testing purposes.
   *
   * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
   */
  virtual void setIgnoreSslPeerVerification(bool /*ignoreSslPeerVerification*/)
  {
    mooseWarning("You are attempting to execute setIgnoreSslPeerVerification "
                 "but did not specify a valid ICE Updater INetworkingTool!"
                 "Doing nothing.");
  }

  /**
   * Sets the noProxyFlag's value to 'val'.
   *
   * @param val The new value for the noProxyFlag.
   */
  virtual void setNoProxyFlag(bool /*val*/)
  {
    mooseWarning("You are attempting to execute setNoProxyFlag "
                 "but did not specify a valid ICE Updater INetworkingTool!"
                 "Doing nothing.");
  }
};

#endif
