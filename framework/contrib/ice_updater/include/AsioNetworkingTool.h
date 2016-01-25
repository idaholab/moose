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
#ifndef ASIONETWORKINGTOOL_H_
#define ASIONETWORKINGTOOL_H_

#include "INetworkingTool.h"
#include "MooseTypes.h"

#ifdef ASIO_STANDALONE
#include "asio.hpp"

/**
 * The AsioNetworkignTool is a realization of the INetworkingTool
 * interface that uses the stand-alone Asio library to post/get
 * http requests.
 */
class AsioNetworkingTool: public INetworkingTool
{
private:
  /**
   * Reference to the io_service used by the client socket.
   */
  MooseSharedPointer<asio::io_service> io_service;

  /**
   * Reference to the client socket used in communicating with
   * the remote server.
   */
  MooseSharedPointer<asio::ip::tcp::socket> socket;

public:
  /**
   * The constructor
   */
  AsioNetworkingTool();

  /**
   * The destructor
   */
  ~AsioNetworkingTool();

  /**
   * Use Asio Library to perform HTTP GET to return the contents located at url.
   *
   * @param url The URL of the GET request.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @return The contents at the URL or an error message if one took place.
   */
  std::string get(std::string url,
                  std::string username,
                  std::string password);

  /**
   * Use Asio library to perform HTTP POST to transmit value at url.
   *
   * @param url The url that is used to post the value.
   * @param username The username. It is ignored if it is empty. It may not be null.
   * @param password The password. It is ignored if it is empty. It may not be null.
   * @param value The value that is posted to the url.
   * @return A std::string containing the error if one took place. Else returns an empty std::string.
   */
  std::string post(std::string url,
                   std::string value,
                   std::string username,
                   std::string password);

  /**
   * Sets the ignoreSslPeerVerification flag. If ignoreSslPeerVerification flag is
   * set to true then cURL will skip peer certificate verification for HTTPS urls.
   * This flag should only be set to true for testing purposes.
   *
   * @param ignoreSslPeerVerification The value for the ignoreSslPeerVerification flag.
   */
  virtual void setIgnoreSslPeerVerification(bool ignoreSslPeerVerification)
  {
    return;
  }

  /**
   * Sets the noProxyFlag's value to 'val'.
   *
   * @param val The new value for the noProxyFlag.
   */
  virtual void setNoProxyFlag(bool val)
  {
    return;
  }
};

#else // !ASIO_STANDALONE

/**
 * If we aren't using cxx11, build a stub AsioNetworkingTool class that does nothing
 * but throw errors if used.
 */
class AsioNetworkingTool
{
public:
  /**
   * The constructors all throw errors.
   */
  AsioNetworkingTool() { mooseError("Asio Networking Tool requires --enable-cxx11 parameter to update_and_build_libmesh.sh."); }
  AsioNetworkingTool(std::istream & /*stream*/) { mooseError("Asio Networking Tool requires --enable-cxx11 parameter to update_and_build_libmesh.sh."); }

  /**
   * The following functions do nothing, and will never be called.
   */
  std::string get(std::string /*url*/,
                  std::string /*username*/,
                  std::string /*password*/)
    { return std::string(""); }

  std::string post(std::string /*url*/,
                   std::string /*value*/,
                   std::string /*username*/,
                   std::string /*password*/)
    { return std::string(""); }
};

#endif // ASIO_STANDALONE

#endif
