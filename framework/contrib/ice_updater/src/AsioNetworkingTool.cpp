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
#include "AsioNetworkingTool.h"

#ifdef ASIO_STANDALONE

using namespace asio;

static const std::string base64_chars =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

std::string base64_encode(char const* bytes_to_encode, int in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
        + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
        + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
      + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
      + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

return ret;
}

std::vector<std::string> & splitStr(const std::string & s,
                                    char delim,
                                    std::vector<std::string> & elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
    elems.push_back(item);
  return elems;
}

std::vector<std::string> splitStr(const std::string & s, char delim)
{
  std::vector<std::string> elems;
  splitStr(s, delim, elems);
  return elems;
}

AsioNetworkingTool::AsioNetworkingTool()
{
  io_service = MooseSharedPointer<asio::io_service>(new asio::io_service());
  socket = MooseSharedPointer<ip::tcp::socket>(new ip::tcp::socket(*io_service));
}

AsioNetworkingTool::~AsioNetworkingTool()
{
  socket->close();
  io_service->stop();
}

std::string AsioNetworkingTool::get(std::string /*url*/,
                                    std::string /*username*/,
                                    std::string /*password*/)
{
  // To be implemented
  return "";
}

std::string AsioNetworkingTool::post(std::string /*url*/,
                                     std::string /*value*/,
                                     std::string /*username*/,
                                     std::string /*password*/)
{

  // Local error code
  asio::error_code errorCode;

  // Get the hostname and the port
  if (url.find("http://") != std::string::npos)
  {
  url = url.erase(0, std::string("http://").size());
  }
  // Get rid of the /ice/update...
  int index = url.find_first_of("/");
  url = url.erase(index, url.size());

  // Split to get the host and port
  std::vector<std::string> hostAndPort = splitStr(url, ':');

  // if the socket isn't open, then open it
  if (!socket->is_open())
  {

    // Create the EndPoint and Connect the socket
    ip::tcp::endpoint iceEndPoint(ip::address::from_string(hostAndPort[0]=="localhost" ? "127.0.0.1" : hostAndPort[0]), std::stoi(hostAndPort[1]));
    socket->connect(iceEndPoint, errorCode);

    if (errorCode)
    {
      return "Error in connecting the socket to " + url + ".\n";
    }
  }

  if (socket->is_open())
  {
    // Start the message construction
    std::string message = "post=" + value;
    std::string uNamePassword = "ice:veryice";
    std::string credentials = base64_encode(uNamePassword.c_str(),
                                            uNamePassword.length());

    // Form the request
    streambuf request;
    std::ostream request_stream(&request);
    request_stream << "POST /ice/update HTTP/1.1\r\n";
    request_stream << "Host:" << hostAndPort[0] + ":" + hostAndPort[1] << "\r\n";
    request_stream << "Authorization: Basic " + credentials + "\r\n";
    request_stream << "User-Agent: C/1.0";
    request_stream << "Accept: */*\r\n";
    request_stream << "Content-Length: " << message.length() + 2 << "\r\n";
    request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
    request_stream << "\r\n\r\n";  //NOTE THE Double line feed
    request_stream << message;

    // Write the message to the socket.
    asio::write(*socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    streambuf response;
    int len = asio::read_until(*socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      return "Invalid response " + http_version + ", " + std::to_string(status_code) + ".";
    }

    if (status_code != 200)
    {
      return "Response returned with status code " + std::to_string(status_code) + ".";
    }

    // Wait a little bit to ensure anyone sending
    // gets their chance.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Close this socket to reset it.
    socket->close();

    return "";

  } else
  {
    return "Asio Error: Could not open socket to " + url + ".";
  }

}

#endif
