# tinyhttp
A tiny HTTP server library written in modern C++ with REST and WS support, used mostly in my embedded projects.

## Motivation

I love to use C++ wherever I can, and I wanted to implement a HTTP serverver using my C++17 knowledge.

## How to use

If you want an environment to tinker around a bit, check out the examples folder :)

### Creating a server

```c++
HttpServer server;

// Setup your handlers here...

server.startListening(80); // Start listening on port 80
```

*Currently specifying listen address is not supported.*

### Serving static files

```c++

// Setup root document
server.when("/")
    // Answer with a specific file
    ->serveFile("/path/to/index.html");

// Setup folder for static files
server.whenMatching("/static/[^/]+")
    // Answer with a file from a folder
    ->serveFromFolder("/path/to/static/files/");
```

As of now `serveFromFolder` does not support subdirectories for security reasons.

*Currently specifying custom MIME types is not supported. The MIME type is guessed from the file extension.*

### Custom handlers

Here we define an endpoint (`/mynumber`) that stores an integer.

```c++
static int myNumber = 0;

server.when("/mynumber")
    // Handle when data is posted here (POST)
    ->posted([](const HttpRequest& req) {
        // Set the number by parsing the raw request body
        myNumber = atoi(req.content().c_str());
        return HttpResponse{200};
    })
    // Handle when data is requested from here (GET)
    ->requested([](const HttpRequest& req) {
        // Reply with a plain text response containing the stored number
        return HttpResponse{200, "text/plain", std::to_string(myNumber)};
    });
```

### Working with json

I used [MiniJson](https://github.com/zsmj2017/MiniJson) because it was tiny and easy-to use. Here is an implementation of the same functionality as in the previous example, but with JSON.

```c++
static int myNumber = 0;

server.when("/mynumber/json")
    // Handle when data is posted here (POST)
    ->posted([](const HttpRequest& req) {
        const auto& data = req.json();

        // If the parsed data is not an object, ignore
        if (!data.isObject())
            return HttpResponse{400, "text/plain", "Invalid JSON"};

        const auto& jNumber = data["value"];

        // Check if the data with the key "value" is a number
        if (!jNumber.isNumber())
            return HttpResponse{400, "text/plain", "Value is not a number"};

        // Set the number by parsing the raw request body
        myNumber = static_cast<int>(jNumber.toDouble());
        return HttpResponse{200};
    })
    // Handle when data is requested from here (GET)
    ->requested([](const HttpRequest& req) {
        // Create a new JSON object
        miniJson::Json::_object obj;

        // Add a new key "value" to it, set to myNumber
        obj["value"] = myNumber;

        // Respond with json
        return HttpResponse{200, obj};
    });
```

### Protocol handover

This feature is used internally by the WebSocket API, but available for implementing other protocols, like MJPEG streams. The following example outlines how to write a simple MJPEG handler.

```c++

// Function that generates the content of a frame
std::string getFrame() {
    std::vector<unsigned char> buff;
    // fill buff with JPEG data

    std::string content;

    // x-mixed-replace boundary marker
    content += "--FRAME\r\n";

    // Content-Length of this part
    content += "Content-Length: " + std::to_string(buff.size()) + "\r\n\r\n";

    // You can add Content-Type as well, but browsers can figure this out

    // Write the actual data
    content.append((char*)buff.data(), buff.size());

    // Close the frame with a newline
    content += "\r\n";

    return content;
}

// Handler for protocol handover
struct MyStreamer : public ICanRequestProtocolHandover {
    void acceptHandover(short& serverSock, IClientStream& client, std::unique_ptr<HttpRequest> srcRequest) {
        int res;

        // Accept while both the client and server sockets are valid
        while (serverSock > 0 && client.isOpen()) {
            std::string content = getFrame();

            // Send frame
            client.send(content.data(), content.size());

            usleep(40000); // push 25 Frame/s
        }
    }
};

// ...

static MyStreamer myStreamHandler;

server.when("/video.mjpeg")->requested([](const HttpRequest& req) {
    HttpResponse res{200};

    // Set headers to disable caching
    res["Age"]              = "0";
    res["Cache-Control"]    = "no-cache, private";
    res["Pragma"]           = "no-cache";

    // Set content type to x-mixed-replace, with "FRAME" as boundary marker
    res["Content-Type"]     = "multipart/x-mixed-replace;boundary=FRAME";

    // Remove the Content-Length header
    res["Content-Length"] = "";

    // Request protocol handover after this response is sent
    res.requestProtocolHandover(&myStreamHandler);
    return res;
});

```

### Websockets

The current WebSocket implementation is experimental, and not 100% complete. **Use it on your own risk.**

```c++

// Define a handler class for WebSocket sessions, each connection will have its own instance of this class
struct MyWebsockHandler : public WebsockClientHandler {
    void onConnect() override {
        puts("Connect!");
    }

    void onTextMessage(const std::string& message) override {
        puts("Text message!");
    }

    void onBinaryMessage(const uint8_t* message, const size_t len) override {
        puts("Binary message!");
    }

    void onDisconnect() override {
        puts("Disconnect!");
    }
};

// ...

// Register our handler with the path /ws
server.websocket("/ws")->handleWith<MyWebsockHandler>();

```
