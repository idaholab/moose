#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

// json support (Currently uses MiniJson)
#define TINYHTTP_JSON

// websocket support
// #define TINYHTTP_WS

// template integration
// #define TINYHTTP_TEMPLATES

// threading support
#define TINYHTTP_THREADING

// allow keep-alive connections
// (you should disable this if you are using a single thread)
// #define TINYHTTP_ALLOW_KEEPALIVE

#ifndef MAX_HTTP_HEADERS
#define MAX_HTTP_HEADERS 30
#endif

#ifndef MAX_HTTP_CONTENT_SIZE
#define MAX_HTTP_CONTENT_SIZE (50 * 1024) // 50kiB
#endif

#ifndef MAX_ALLOWED_WS_FRAME_LENGTH
#define MAX_ALLOWED_WS_FRAME_LENGTH (50 * 1024) // 50kiB
#endif

#ifndef WS_FRAGMENT_THRESHOLD
#define MAX_ALLOWED_WS_FRAME_LENGTH (50 * 1024) // 50kiB
#endif

// Disabled if set to a <= 0 value
// Timeout for regular clients keep-alive connections
// (Ignored for socket takeovers like WebSockets)
#ifndef TINYHTTP_CLIENT_TIMEOUT
#define TINYHTTP_CLIENT_TIMEOUT (30) // Seconds
#endif

#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex>
#include <map>
#include <iostream>
#include <fstream>
#include <list>
#include <chrono>
#include <variant>

#ifdef TINYHTTP_THREADING
#include <thread>
#include <mutex>
#endif

#ifdef TINYHTTP_JSON
#include "minijson/minijson.h"
#endif

#ifdef TINYHTTP_TEMPLATES
#include <HTMLTemplate.h>
#endif

enum class HttpRequestMethod { GET,POST,PUT,DELETE,OPTIONS,UNKNOWN };

#ifdef TINYHTTP_WS
enum {
    WSOPC_CONTINUATION  = 0x0,
    WSOPC_TEXT          = 0x1,
    WSOPC_BINARY        = 0x2,
    WSOPC_NONCTRL_RES0  = 0x3,
    WSOPC_NONCTRL_RES1  = 0x4,
    WSOPC_NONCTRL_RES2  = 0x5,
    WSOPC_NONCTRL_RES3  = 0x6,
    WSOPC_NONCTRL_RES4  = 0x7,
    WSOPC_DISCONNECT    = 0x8,
    WSOPC_PING          = 0x9,
    WSOPC_PONG          = 0xA,
    WSOPC_CTRL_RES0     = 0xB,
    WSOPC_CTRL_RES1     = 0xC,
    WSOPC_CTRL_RES2     = 0xD,
    WSOPC_CTRL_RES3     = 0xE,
    WSOPC_CTRL_RES4     = 0xF,
};
#endif

struct IClientStream {
    virtual ~IClientStream() = default;
    virtual bool isOpen() noexcept = 0;
    virtual void send(const void* what, size_t size) = 0;
    virtual size_t receive(void* target, size_t max) = 0;
    virtual std::string receiveLine(bool asciiOnly = true, size_t max = -1) = 0;
    virtual void close() = 0;

    // wrapper for send for any object having a data() -> uint8_t* and a size() -> integer function
    template<
        typename T,
        typename Chk1 = typename std::enable_if<std::is_same<decltype(T().data()), uint8_t*>::value>::type,
        typename Chk2 = typename std::enable_if<std::is_integral<decltype(T().size())>::value>::type
    >
    void send(const T& data) { send(data.data(), data.size()); }

    bool mErrorFlag = false;
};

class TCPClientStream : public IClientStream {
    int mSocket;

    public:
        ~TCPClientStream() { close(); }
        TCPClientStream(short socket) : mSocket{socket} {}
        TCPClientStream(const TCPClientStream&) = delete;
        TCPClientStream(TCPClientStream&& other) : mSocket{other.mSocket} { other.mSocket = -1; }

        static TCPClientStream acceptFrom(short listener);

        bool isOpen() noexcept override { return mSocket >= 0 && !mErrorFlag; }
        void send(const void* what, size_t size) override;
        size_t receive(void* target, size_t max) override;
        std::string receiveLine(bool asciiOnly = true, size_t max = -1) override;
        void close() override;
};

struct StdinClientStream : IClientStream {
    bool isOpen() noexcept override { return true; };
    void send(const void* what, size_t size) override {
        fwrite(what, 1, size, stdout);
        fflush(stdout);
    }
    size_t receive(void* target, size_t max) override {
        return fread(target, 1, max, stdin);
    }
    std::string receiveLine(bool asciiOnly = true, size_t max = -1) override {
        (void)asciiOnly;
        (void)max;
        std::string res;
        std::getline(std::cin, res);

        if (res.length() > 0 && res[res.length()-1] == '\r')
            res = res.substr(0, res.length() - 1);

        return res;
    }
    void close() override {}
};

struct MessageBuilder : public std::vector<uint8_t> {
    template<typename T>
    void write(T s) {
        write(&s, sizeof(T));
    }

    void write(const std::string& s) {
        write(s.data(), s.size());
    }

    void write(const char* s) {
        write(s, strlen(s));
    }

    void writeCRLF() { write("\r\n", 2); }

    void write(const void* data, const size_t len) {
        size_t ogsize = size();
        resize(ogsize + len);
        memcpy(this->data() + ogsize, data, len);
    }
};

class HttpMessageCommon {
    protected:
        std::map<std::string, std::string> mHeaders;
        std::string mContent;

    public:
        std::string& operator[](std::string i) {
            std::transform(i.begin(), i.end(), i.begin(), [](unsigned char c){ return std::tolower(c); });

            auto f = mHeaders.find(i);
            if (f == mHeaders.end()) {
                if (mHeaders.size() >= MAX_HTTP_HEADERS)
                    throw std::runtime_error("too many HTTP headers");

                mHeaders.insert(std::pair<std::string,std::string>(i, ""));
            }

            return mHeaders.find(i)->second;
        }

        std::string operator[](std::string i) const {
            std::transform(i.begin(), i.end(), i.begin(), [](unsigned char c){ return std::tolower(c); });

            auto f = mHeaders.find(i);
            if (f == mHeaders.end())
                return "";

            return mHeaders.find(i)->second;
        }

        void setContent(std::string content) {
            mContent = std::move(content);
            (*this)["Content-Length"] = std::to_string(mContent.size());
        }

        const auto& content() const noexcept { return mContent; }
};

class HttpRequest : public HttpMessageCommon {
    HttpRequestMethod mMethod = HttpRequestMethod::UNKNOWN;
    std::string path, query;

    #ifdef TINYHTTP_JSON
    miniJson::Json mContentJson;
    #endif

    public:
        bool parse(std::shared_ptr<IClientStream> stream);

        const HttpRequestMethod& getMethod() const noexcept { return mMethod; }
        const std::string& getPath() const noexcept { return path; }
        const std::string& getQuery() const noexcept { return query; }

        #ifdef TINYHTTP_JSON
        const miniJson::Json& json() const noexcept { return mContentJson; }
        #endif
};

struct ICanRequestProtocolHandover {
    virtual ~ICanRequestProtocolHandover() = default;
    virtual void acceptHandover(int& serverSock, IClientStream& client, std::unique_ptr<HttpRequest> srcRequest) = 0;
};

#ifdef TINYHTTP_WS
struct WebsockClientHandler {
    virtual void onConnect() {}
    virtual void onDisconnect() {}
    virtual void onTextMessage(const std::string& message) {}
    virtual void onBinaryMessage(const std::vector<uint8_t>& data) {}

    void sendRaw(uint8_t opcode, const void* data, size_t length, bool mask = false);
    void sendDisconnect();
    void sendText(const std::string& str);
    void sendBinary(const void* data, size_t length);

    template<size_t N>
    inline void sendBinary(const uint8_t data[N]) {
        sendBinary(data, N);
    }

    // wrapper for send for any object having a data() -> uint8_t* and a size() -> integer function
    template<
        typename T,
        typename Chk1 = typename std::enable_if<std::is_same<decltype(T().data()), uint8_t*>::value>::type,
        typename Chk2 = typename std::enable_if<std::is_integral<decltype(T().size())>::value>::type
    >
    void sendBinary(const T& data) { sendBinary(data.data(), data.size()); }

    #ifdef TINYHTTP_JSON

    inline void sendJson(const miniJson::Json& json) {
        sendText(json.serialize());
    }

    #endif

    void attachTcpStream(IClientStream* s) { mClient = s; }
    void attachRequest(std::unique_ptr<HttpRequest> req) { mRequest.swap(req); }

    protected:
        IClientStream* mClient;
        std::unique_ptr<HttpRequest> mRequest;
};
#endif

class HttpResponse : public HttpMessageCommon {
    unsigned mStatusCode = 400;
    ICanRequestProtocolHandover* mHandover = nullptr;

    public:
        HttpResponse(const unsigned statusCode) : mStatusCode{statusCode} {
            (*this)["Server"] = "tinyHTTP_1.1";

            if (statusCode >= 200)
                (*this)["Content-Length"] = "0";
        }

        HttpResponse(const unsigned statusCode, std::string contentType, std::string content)
            : HttpResponse{statusCode} {
            (*this)["Content-Type"] = contentType;
            setContent(content);
        }

        inline void requestProtocolHandover(ICanRequestProtocolHandover* newOwner) noexcept {
            mHandover = newOwner;
        }

        inline bool acceptProtocolHandover(ICanRequestProtocolHandover** outTarget) noexcept {
            if (outTarget && mHandover)
            {
                *outTarget = mHandover;
                return true;
            }

            return false;
        }

        #ifdef TINYHTTP_JSON
        HttpResponse(const unsigned statusCode, const miniJson::Json& json)
            : HttpResponse{statusCode, "application/json", json.serialize()} {}
        #endif

        #ifdef TINYHTTP_TEMPLATES
        HttpResponse(const unsigned statusCode, HTMLTemplate&& _template)
            : HttpResponse{statusCode, "text/html", _template.render()} {}
        #endif

        MessageBuilder buildMessage() {
            MessageBuilder b;

            b.write("HTTP/1.1 " + std::to_string(mStatusCode));
            b.writeCRLF();

            for (auto& h : mHeaders)
                if (!h.second.empty())
                    b.write(h.first + ": " + h.second + "\r\n");

            b.writeCRLF();
            b.write(mContent);

            return b;
        }
};

struct HandlerBuilder {
    virtual ~HandlerBuilder() = default;

    virtual std::unique_ptr<HttpResponse> process(const HttpRequest& req) {
        (void)req;
        return nullptr;
    }
};

#ifdef TINYHTTP_WS
class WebsockHandlerBuilder : public HandlerBuilder, public ICanRequestProtocolHandover {
    struct Factory {
		virtual ~Factory() = default;
        virtual WebsockClientHandler* makeInstance() = 0;
    };

    template<typename T>
    struct FactoryT : Factory {
        virtual WebsockClientHandler* makeInstance() {
            return new T();
        }
    };

    std::unique_ptr<Factory> mFactory;

    public:
        WebsockHandlerBuilder()
            : mFactory{new FactoryT<WebsockClientHandler>} {}

        template<typename T>
        void handleWith() {
            mFactory = std::unique_ptr<Factory>(new FactoryT<T>);
        }

        virtual std::unique_ptr<HttpResponse> process(const HttpRequest& req) override;

        void acceptHandover(int& serverSock, IClientStream& client, std::unique_ptr<HttpRequest> srcRequest) override;
};
#endif

class HttpHandlerBuilder : public HandlerBuilder {
    typedef std::function<HttpResponse(const HttpRequest&)> HandlerFunc;

    std::map<HttpRequestMethod, HandlerFunc> mHandlers;

    static bool isSafeFilename(const std::string& name, bool allowSlash);
    static std::string getMimeType(std::string name);

    public:
        HttpHandlerBuilder* posted(HandlerFunc h) {
            mHandlers.insert(std::pair<HttpRequestMethod, HandlerFunc>(HttpRequestMethod::POST, std::move(h)));
            return this;
        }

        HttpHandlerBuilder* requested(HandlerFunc h) {
            mHandlers.insert(std::pair<HttpRequestMethod, HandlerFunc>(HttpRequestMethod::GET, std::move(h)));
            return this;
        }

        HttpHandlerBuilder* serveFile(std::string name) {
            return requested([name](const HttpRequest&) {
                std::ifstream t(name);
                if (t.is_open())
                    return HttpResponse{200, getMimeType(name), std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>())};
                else {
                    std::cerr << "Could not locate file: " << name << std::endl;
                    return HttpResponse{404, "text/plain", "The requested file is missing from the server"};
                }
            });
        }

        HttpHandlerBuilder* serveFromFolder(std::string dir) {
            return requested([dir](const HttpRequest&q) {
                std::string fname = q.getPath();
                fname = fname.substr(fname.rfind('/')+1);

                if (isSafeFilename(fname, false)) {
                    fname = dir + "/" + fname;

                    std::ifstream t(fname);
                    if (t.is_open())
                        return HttpResponse{200, getMimeType(fname), std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>())};
                    else
                        std::cerr << "Could not locate file: " << fname << std::endl;
                }

                return HttpResponse{404, "text/plain", "The requested file is missing from the server"};
            });
        }

        template<typename T>
        inline HttpHandlerBuilder* posted(T x) {
            return posted(HandlerFunc(std::move(x)));
        }

        template<typename T>
        inline HttpHandlerBuilder* requested(T x) {
            return requested(HandlerFunc(std::move(x)));
        }

        std::unique_ptr<HttpResponse> process(const HttpRequest& req) override {
            auto h = mHandlers.find(req.getMethod());

            if (h == mHandlers.end())
                return std::make_unique<HttpResponse>(405, "text/plain", "405 method not allowed");
            else
                return std::make_unique<HttpResponse>(h->second(req));
        }
};

class HttpServer {
    std::vector<std::pair<std::string, std::shared_ptr<HandlerBuilder>>> mHandlers;
    std::vector<std::pair<std::regex, std::shared_ptr<HandlerBuilder>>> mReHandlers;
    MessageBuilder mDefault404Message, mDefault400Message;
    int mSocket = -1;
    bool mCleanupThreadShutdown = false;

    std::shared_ptr<HttpResponse> processRequest(std::string key, const HttpRequest& req) {
        try {
            for (auto& x : mHandlers)
                if (x.first == key) {
                    auto res = x.second->process(req);
                    if (res) return res;
                }

            for (auto x : mReHandlers)
                if (std::regex_match(key, x.first)) {
                    auto res = x.second->process(req);
                    if (res) return res;
                }
        } catch (std::exception& e) {
            std::cerr << "Exception while handling request (" << key << "): " << e.what() << std::endl;
            return std::make_shared<HttpResponse>(500, "text/plain", "500 exception while processing");
        }

        return nullptr;
    }

    class Processor : public std::enable_shared_from_this<Processor> {
        std::shared_ptr<IClientStream> mClientStream;
        HttpServer& mOwner;
        std::chrono::system_clock::time_point mLastActive;
        bool mIsAlive, mHasHandover;

        #ifdef TINYHTTP_THREADING
        std::unique_ptr<std::thread> mWorkThread;
        std::mutex mShutdownMutex;
        #endif

        public:
            static void clientThreadProc(std::shared_ptr<Processor> self);

            Processor(std::shared_ptr<IClientStream> stream, HttpServer& owner);

            ~Processor() {
                shutdown();
            }

            inline bool isAlive() const noexcept {
                return mIsAlive;
            }
            bool isTimedOut() const noexcept;
            void shutdown();

            #ifdef TINYHTTP_THREADING
            void startThread();
            #endif
    };

    #ifdef TINYHTTP_THREADING
    void cleanupThreadProc();

    std::unique_ptr<std::thread> mCleanupThread;
    std::list<std::shared_ptr<Processor>> mRequestProcessors;
    std::mutex mRequestProcessorListMutex;
    #else
    std::shared_ptr<Processor> mCurrentProcessor;
    #endif

    public:
        HttpServer();
        ~HttpServer() {
            mCleanupThreadShutdown = true;
            shutdown();

            #ifdef TINYHTTP_THREADING
            if (mCleanupThread->joinable())
                mCleanupThread->join();
            #endif
        }

#ifdef TINYHTTP_WS
        std::shared_ptr<WebsockHandlerBuilder> websocket(std::string path) {
            auto h = std::make_shared<WebsockHandlerBuilder>();
            mHandlers.insert(mHandlers.begin(), std::pair<std::string, std::shared_ptr<WebsockHandlerBuilder>>{std::move(path), h});
            return h;
        }
#endif

        std::shared_ptr<HttpHandlerBuilder> when(std::string path) {
            auto h = std::make_shared<HttpHandlerBuilder>();
            mHandlers.push_back(std::pair<std::string, std::shared_ptr<HttpHandlerBuilder>>{std::move(path), h});
            return h;
        }

        std::shared_ptr<HttpHandlerBuilder> whenMatching(std::string path) {
            auto h = std::make_shared<HttpHandlerBuilder>();
            mReHandlers.push_back(std::pair<std::regex, std::shared_ptr<HttpHandlerBuilder>>{std::regex{std::move(path)}, h});
            return h;
        }

        void startListening(const std::variant<uint16_t, std::string> listen_on);
        void shutdown();
};

#endif
