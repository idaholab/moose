
#include "http.h"

#include <vector>
#include <sstream>

/*static*/ TCPClientStream
TCPClientStream::acceptFrom(short listener)
{
  struct sockaddr_in client;
  const size_t clientLen = sizeof(client);

  short sock = accept(listener,
                      reinterpret_cast<struct sockaddr *>(&client),
                      const_cast<socklen_t *>(reinterpret_cast<const socklen_t *>(&clientLen)));

  if (sock < 0)
  {
    perror("accept failed");
    return {-1};
  }

  return {sock};
}

void
TCPClientStream::send(const void * what, size_t size)
{
  if (::send(mSocket, what, size, MSG_NOSIGNAL) < 0)
    throw std::runtime_error("TCP send failed");
}

size_t
TCPClientStream::receive(void * target, size_t max)
{
  ssize_t len;

  if ((len = recv(mSocket, target, max, MSG_NOSIGNAL)) < 0)
    throw std::runtime_error("TCP receive failed");

  return static_cast<size_t>(len);
}

std::string
TCPClientStream::receiveLine(bool asciiOnly, size_t max)
{
  std::string res;
  char ch;

  while (res.size() < max)
  {
    if (recv(mSocket, &ch, 1, MSG_NOSIGNAL) != 1)
      throw std::runtime_error("TCP receive failed");

    if (ch == '\r')
      continue;
    if (ch == '\n')
      break;

    if (asciiOnly && !isascii(ch))
      throw std::runtime_error("Only ASCII characters were allowed");

    res.push_back(ch);
  }

  return res;
}

void
TCPClientStream::close()
{
  if (mSocket < 0)
    return;
  ::close(mSocket);
  mSocket = -1;
}

bool
HttpRequest::parse(std::shared_ptr<IClientStream> stream)
{
  std::istringstream iss(stream->receiveLine());
  std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                   std::istream_iterator<std::string>());

  if (results.size() < 2)
    return false;

  std::string methodString = results[0];
  if (methodString == "GET")
  {
    mMethod = HttpRequestMethod::GET;
  }
  else if (methodString == "POST")
  {
    mMethod = HttpRequestMethod::POST;
  }
  else if (methodString == "PUT")
  {
    mMethod = HttpRequestMethod::PUT;
  }
  else if (methodString == "DELETE")
  {
    mMethod = HttpRequestMethod::DELETE;
  }
  else if (methodString == "OPTIONS")
  {
    mMethod = HttpRequestMethod::OPTIONS;
  }
  else
    return false;

  path = results[1];

  ssize_t question = path.find("?");
  if (question > 0)
  {
    query = path.substr(question);
    path = path.substr(0, question);
  }

  /*
  if (query.empty())
      std::co ut << methodString << " " << path << std::endl;
  else
      std::co ut << methodString << " " << path << " (Query: " << query << ")" << std::endl;
  */

  while (true)
  {
    std::string line = stream->receiveLine();

    if (line.empty())
      break;

    ssize_t sep = line.find(": ");
    if (sep <= 0)
      return false;

    std::string key = line.substr(0, sep), val = line.substr(sep + 2);
    (*this)[key] = val;
    // std::co ut << "HEADER: <" << key << "> set to <" << val << ">" << std::endl;
  }

  std::string contentLength = (*this)["Content-Length"];
  ssize_t cl = std::atoll(contentLength.c_str());

  if (cl > MAX_HTTP_CONTENT_SIZE)
    throw std::runtime_error("request too large");

  if (cl > 0)
  {
    char * tmp = new char[cl];
    bzero(tmp, cl);
    stream->receive(tmp, cl);

    mContent = std::string(tmp, cl);
    delete[] tmp;

#ifdef TINYHTTP_JSON
    if ((*this)["Content-Type"] == "application/json" ||
        (*this)["Content-Type"].rfind("application/json;", 0) ==
            0 // some clients gives us extra data like charset
    )
    {
      std::string error;
      mContentJson = miniJson::Json::parse(mContent, error);
      if (!error.empty())
        std:: cerr << "Content type was JSON but we couldn't parse it! " << error << std::endl;
    }
#endif
  }

  return true;
}

/*static*/ bool
HttpHandlerBuilder::isSafeFilename(const std::string & name, bool allowSlash)
{
  static const char allowedChars[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-+@";
  for (auto x : name)
  {
    if (x == '/' && !allowSlash)
      return false;

    bool ok = false;
    for (size_t i = 0; allowedChars[i] && !ok; i++)
      ok = allowedChars[i] == x;

    if (!ok)
      return false;
  }

  return true;
}

/*static*/ std::string
HttpHandlerBuilder::getMimeType(std::string name)
{
  static std::map<std::string, std::string> mMimeDatabase;

  if (mMimeDatabase.empty())
  {
    mMimeDatabase.insert({"js", "application/javascript"});
    mMimeDatabase.insert({"pdf", "application/pdf"});
    mMimeDatabase.insert({"gz", "application/gzip"});
    mMimeDatabase.insert({"xml", "application/xml"});
    mMimeDatabase.insert({"html", "text/html"});
    mMimeDatabase.insert({"htm", "text/html"});
    mMimeDatabase.insert({"css", "text/css"});
    mMimeDatabase.insert({"txt", "text/plain"});
    mMimeDatabase.insert({"png", "image/png"});
    mMimeDatabase.insert({"jpg", "image/jpeg"});
    mMimeDatabase.insert({"jpeg", "image/jpeg"});
    mMimeDatabase.insert({"json", "application/json"});
  }

  ssize_t pos = name.rfind(".");
  if (pos < 0)
    return "application/octet-stream";

  auto f = mMimeDatabase.find(name.substr(pos + 1));
  if (f == mMimeDatabase.end())
    return "application/octet-stream";

  return f->second;
}

HttpServer::HttpServer()
{
  mDefault404Message = HttpResponse{404, "text/plain", "404 not found"}.buildMessage();
  mDefault400Message = HttpResponse{400, "text/plain", "400 bad request"}.buildMessage();
}

void
HttpServer::startListening(uint16_t port)
{
#ifndef TINYHTTP_FUZZING
  mSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (mSocket == -1)
    throw std::runtime_error("Could not create socket");

  struct sockaddr_in remote;

  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = htonl(INADDR_ANY);
  remote.sin_port = htons(port);
  int iRetval;

  while (true)
  {
    iRetval = bind(mSocket, reinterpret_cast<struct sockaddr *>(&remote), sizeof(remote));

    if (iRetval < 0)
    {
      perror("Failed to bind socket, retrying in 5 seconds...");
      usleep(1000 * 5000);
    }
    else
      break;
  }

  listen(mSocket, 3);
#else
  mSocket = 0;
#endif

  printf("Waiting for incoming connections...\n");
  while (mSocket != -1)
  {
#ifdef TINYHTTP_FUZZING
    auto stream = std::make_shared<StdinClientStream>();
#else
    auto stream =
        std::shared_ptr<IClientStream>(new TCPClientStream{TCPClientStream::acceptFrom(mSocket)});
#endif

    std::thread th(
        [stream, this]()
        {
          ICanRequestProtocolHandover * handover = nullptr;

          std::unique_ptr<HttpRequest> handoverRequest;

          try
          {
            while (stream->isOpen())
            {
              HttpRequest req;

              try
              {
                if (!req.parse(stream))
                {
                  stream->send(mDefault400Message);
                  stream->close();
                  continue;
                }
              }
              catch (...)
              {
                stream->send(mDefault400Message);
                stream->close();
                continue;
              }

              auto res = processRequest(req.getPath(), req);
              if (res)
              {
                auto builtMessage = res->buildMessage();
                stream->send(builtMessage);

                if (res->acceptProtocolHandover(&handover))
                {
                  handoverRequest = std::make_unique<HttpRequest>(req);
                  break;
                }

                goto keep_alive_check;
              }

              stream->send(mDefault404Message);

            keep_alive_check:
              if (req["Connection"] != "keep-alive")
                break;
            }

            if (handover)
              handover->acceptHandover(mSocket, *stream.get(), std::move(handoverRequest));
          }
          catch (std::exception & e)
          {
            std:: cerr << "Exception in HTTP client handler (" << e.what() << ")\n";
          }

          stream->close();
        });

#ifdef TINYHTTP_FUZZING
    th.join();
    break;
#else
    th.detach();
#endif
  }

  puts("Listen loop shut down");
}

void
HttpServer::shutdown()
{
  close(mSocket);
  mSocket = -1;
}
