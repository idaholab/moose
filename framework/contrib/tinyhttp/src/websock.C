// Disabling this for now because we don't need it
#if 0

#include "http.h"

#ifndef TINYHTTP_WS
#warning                                                                                           \
    "You are compiling websock.cpp but you haven't enabled TINYHTTP_WS, please check your build system"
#endif

const char WEBSCOK_MAGIC_UID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const size_t WEBSOCK_MAGIC_UID_LEN = sizeof(WEBSCOK_MAGIC_UID) - 1;

#ifndef SHA1_DIGEST_LENGTH
#define SHA1_DIGEST_LENGTH 20
#endif

static inline constexpr uint32_t leftRotate(const uint32_t n, const uint32_t d) noexcept {
     return (n << d) | (n >> (32-d));
}

#define SHA1_CONVERT_BLOCK(n)                                                                      \
  outBuffer[n * 4 + 0] = (h##n >> 24) & 0xFF;                                                      \
  outBuffer[n * 4 + 1] = (h##n >> 16) & 0xFF;                                                      \
  outBuffer[n * 4 + 2] = (h##n >> 8) & 0xFF;                                                       \
  outBuffer[n * 4 + 3] = (h##n >> 0) & 0xFF;


void hash_sha1(const void* dataptr, const size_t size, uint8_t* outBuffer) {
    const uint8_t*  ptr     = reinterpret_cast<const uint8_t*>(dataptr);
    std::vector<uint8_t> data(ptr, ptr+size);

    uint32_t h0 = 0x67452301,
             h1 = 0xEFCDAB89,
             h2 = 0x98BADCFE,
             h3 = 0x10325476,
             h4 = 0xC3D2E1F0;

    size_t ml = size * 8;

    data.push_back(0x80);
    ml += 1;

    while ((ml % 512) != 448) {
        if ((ml % 8) == 0)
            data.push_back(0);
        ml++;
    }

    uint64_t _ml = static_cast<uint64_t>(size * 8);
    data.push_back((_ml >> 56) & 0xFF);
    data.push_back((_ml >> 48) & 0xFF);
    data.push_back((_ml >> 40) & 0xFF);
    data.push_back((_ml >> 32) & 0xFF);
    data.push_back((_ml >> 24) & 0xFF);
    data.push_back((_ml >> 16) & 0xFF);
    data.push_back((_ml >>  8) & 0xFF);
    data.push_back((_ml >>  0) & 0xFF);

    ml += 64;

    size_t numChunks = ml / 512;

    uint32_t words[80], a, b, c, d, e, f, k, temp;

    for (size_t i = 0; i < numChunks; i++) {
        for (int j = 0; j < 16; j++) {
            words[j]  = static_cast<uint32_t>(data[i*64 + j*4 + 0]) << 24;
            words[j] |= static_cast<uint32_t>(data[i*64 + j*4 + 1]) << 16;
            words[j] |= static_cast<uint32_t>(data[i*64 + j*4 + 2]) <<  8;
            words[j] |= static_cast<uint32_t>(data[i*64 + j*4 + 3]) <<  0;
        }

        for (int j = 16; j < 80; j++)
            words[j] = leftRotate(words[j-3] ^ words[j-8] ^ words[j-14] ^ words[j-16], 1);

        a = h0, b = h1, c = h2, d = h3, e = h4;

        for (int j = 0; j < 80; j++) {
            if (j < 20)
                f = (b & c) | ((~b) & d), k = 0x5A827999;
            else if (j < 40)
                f = b ^ c ^ d, k = 0x6ED9EBA1;
            else if (j < 60)
                f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDC;
            else if (j < 80)
                f = b ^ c ^ d, k = 0xCA62C1D6;

            temp = leftRotate(a, 5) + f + e + k + words[j];
            e = d;
            d = c;
            c = leftRotate(b, 30);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    SHA1_CONVERT_BLOCK(0)
    SHA1_CONVERT_BLOCK(1)
    SHA1_CONVERT_BLOCK(2)
    SHA1_CONVERT_BLOCK(3)
    SHA1_CONVERT_BLOCK(4)
}

// This was a fun afternoon... but it works nicely :)
namespace base64 {
    static inline constexpr char getBase64Char_Impl(const uint8_t idx) noexcept {
        return  idx < 26 ? 'A' + idx         : (
                idx < 52 ? 'a' + idx - 26    : (
                idx < 62 ? '0' + idx - 52    : (
                idx == 62 ? '+' : '/'
        )));
    }

    static inline constexpr char getBase64Char(const uint8_t idx) noexcept {
        return getBase64Char_Impl(idx & 0x3F);
    }

    static inline constexpr uint8_t getBase64Index(char ch) noexcept {
        return  ch == '+' ? 62 : (
                ch == '/' ? 63 : (
                ch <= '9' ? (ch - '0') + 52 : (
                ch == '=' ? 0 : (
                ch <= 'Z' ? (ch - 'A') : (ch - 'a') + 26
        ))));
    }

    static inline std::string encode(const void* dataptr, const size_t size) {
        const uint8_t*  ptr     = reinterpret_cast<const uint8_t*>(dataptr);
        size_t          i;
        std::string     result;

        result.reserve(4 * (size / 3));

        for (i = 0; i < size - (size % 3); i += 3) {
            const int c1 = ptr[i+0],
                      c2 = ptr[i+1],
                      c3 = ptr[i+2];

            result += getBase64Char(c1 >> 2);
            result += getBase64Char(((c1 & 3) << 4) | (c2 >> 4));
            result += getBase64Char(((c2 & 15) << 2) | (c3 >> 6));
            result += getBase64Char(c3);
        }

        auto mod = (size-i) % 3;
        switch (mod) {
            case 1:
                result += getBase64Char(ptr[i] >> 2);
                result += getBase64Char((ptr[i] & 3) << 4);
                result += "==";
                break;
            case 2:
                result += getBase64Char(ptr[i] >> 2);
                result += getBase64Char(((ptr[i] & 3) << 4) | (ptr[i+1] >> 4));
                result += getBase64Char((ptr[i+1] & 15) << 2);
                result += '=';
                break;
            default:
                break;
        }

        return result;
    }

    static inline std::string encode(const std::string& s) {
        return encode(s.data(), s.length());
    }

    static inline std::string decode(const std::string& input) {
        auto            size    = input.length();

        if ((size % 4) != 0)
            return decode(input + "=");

        std::string     result;

        result.reserve((size * 3) / 4);

        for (size_t i = 0; i < size; i += 4) {
            const uint8_t i1 = getBase64Index(input[i+0]),
                          i2 = getBase64Index(input[i+1]),
                          i3 = getBase64Index(input[i+2]),
                          i4 = getBase64Index(input[i+3]);

            result += static_cast<char>((i1 << 2) | (i2 >> 4));
            if (input[i+2] == '=') break;

            result += static_cast<char>((i2 << 4) | (i3 >> 2));
            if (input[i+3] == '=') break;

            result += static_cast<char>((i3 << 6) | i4);
        }

        return result;
    }
}

std::unique_ptr<HttpResponse> WebsockHandlerBuilder::process(const HttpRequest& req) {
    if (req["Connection"].find("Upgrade") != std::string::npos) {
        std::string upgrade = req["Upgrade"];
        if (upgrade != "websocket") {
            fprintf(stderr, "Received connection upgrade with unknown upgrade type: '%s'\n", upgrade.c_str());
            return std::make_unique<HttpResponse>(400); // Send "400 Bad request"
        }

        HttpResponse res{101};
        res["Upgrade"] = "WebSocket";
        res["Connection"] = "Upgrade";

        auto clientKey = req["Sec-WebSocket-Key"];
        if (!clientKey.empty()) {
            std::string accept = clientKey + WEBSCOK_MAGIC_UID;
            unsigned char hash[SHA1_DIGEST_LENGTH];
            hash_sha1(accept.data(), accept.length(), hash);
            res["Sec-WebSocket-Accept"] = base64::encode(hash, SHA1_DIGEST_LENGTH);
        }

        res.requestProtocolHandover(this);
        return std::make_unique<HttpResponse>(res);
    }

    return HandlerBuilder::process(req);
}

void WebsockHandlerBuilder::acceptHandover(short& serverSock, IClientStream& client, std::unique_ptr<HttpRequest> srcRequest) {
    uint8_t buffer[64], realOpc;
    std::vector<uint8_t> contentBuffer;
    bool fin, receivingFragment;

    auto theClient = mFactory->makeInstance();
    theClient->attachTcpStream(&client);
    theClient->attachRequest(std::move(srcRequest));
    theClient->onConnect();

    try {
        while (serverSock > 0 && client.isOpen()) {
            receivingFragment = false;
            contentBuffer.clear();

            size_t totalLength = 0;

            do {
                if (client.receive(buffer, 2) == 0)
                    break;

                uint8_t first  = buffer[0];
                uint8_t second = buffer[1];

                fin = !!(first & 0x80);
                bool msk = !!(second & 0x80);

                if (first & 0x70) {
                    theClient->sendDisconnect();
                    break;
                }

                uint8_t opc = first & 0x0F;

                if (!receivingFragment)
                    realOpc = opc;

                if (receivingFragment && opc != WSOPC_CONTINUATION) {
                    theClient->sendDisconnect();
                    break;
                }

                if (opc == WSOPC_DISCONNECT)
                    break;

                size_t payloadLength = second & 0x7F;
                if (payloadLength == 126) {
                    client.receive(buffer, 2);
                    payloadLength = ntohs(*reinterpret_cast<uint16_t*>(buffer));
                } else if (payloadLength == 127) {
                    client.receive(&payloadLength, 8);
                    payloadLength = be64toh(payloadLength);

                    if (payloadLength & (1ull<<63)) {
                        theClient->sendDisconnect();
                        break;
                    }
                }

                uint32_t key;
                if (msk) {
                    client.receive(&key, 4);
                    key = ntohl(key);
                }

                size_t rl = 0;

                if (totalLength + payloadLength > MAX_ALLOWED_WS_FRAME_LENGTH) {
                    theClient->sendDisconnect();
                    break;
                }

                contentBuffer.resize(totalLength + payloadLength);

                while (rl < payloadLength)
                    rl += client.receive(contentBuffer.data() + totalLength + rl, std::min<size_t>(32768, payloadLength - rl));

                if (msk) {
                    for (size_t i = 0, o = 0; i < payloadLength; i++, o = i % 4) {
                        uint8_t shift = (3 - o) << 3;
                        contentBuffer[i + totalLength] ^= (shift == 0 ? key : (key >> shift)) & 0xFF;
                    }
                }

                totalLength += payloadLength;
                receivingFragment = true;
            } while (!fin);

            switch (realOpc) {
                case WSOPC_TEXT:
                    theClient->onTextMessage(std::string(reinterpret_cast<char*>(contentBuffer.data()), contentBuffer.size()));
                    break;
                case WSOPC_BINARY:
                    theClient->onBinaryMessage(contentBuffer);
                    break;
                case WSOPC_PING:
                    theClient->sendRaw(WSOPC_PONG, contentBuffer.data(), contentBuffer.size());
                    break;
                default:
                    break;
            }
        }
    } catch (std::exception& e) {
        std:: cerr << "WebSocket closed due to an exception (" << e.what() << ")\n";
    }

    theClient->onDisconnect();
}

void WebsockClientHandler::sendRaw(uint8_t opcode, const void* data, size_t length, bool mask) {
    if (!mClient) return;

    size_t bufferPosition = 0, headerPosition;

    size_t allocLen = 2 + std::min<size_t>(length, WS_FRAGMENT_THRESHOLD);

    if (mask)
        allocLen += 4;

    if (WS_FRAGMENT_THRESHOLD > UINT16_MAX)
        allocLen += 8;
    else if (WS_FRAGMENT_THRESHOLD > 126)
        allocLen += 2;

    uint8_t* packetBuffer = new uint8_t[allocLen];

    if (!data)
        length = 0;
    
    const uint8_t* data_u8 = reinterpret_cast<const uint8_t*>(data);
    uint32_t key = static_cast<uint32_t>(rand());

    do {
        headerPosition = 2;
        packetBuffer[0] = opcode & 0xF;
        packetBuffer[1] = 0;

        opcode = WSOPC_CONTINUATION;

        if ((length - bufferPosition) <= WS_FRAGMENT_THRESHOLD)
            packetBuffer[0] |= 0x80; // fin

        if (mask)
            packetBuffer[1] |= 0x80;

        size_t lengthToSend = std::min<size_t>(WS_FRAGMENT_THRESHOLD, (length - bufferPosition));

        if (lengthToSend < 126) {
            packetBuffer[1] |= static_cast<uint8_t>(lengthToSend);
        } else if (lengthToSend <= UINT16_MAX) {
            packetBuffer[1] |= 126;
            *reinterpret_cast<uint16_t*>(&packetBuffer[headerPosition]) = htobe16(static_cast<uint16_t>(lengthToSend));
            headerPosition += 2;
        } else {
            packetBuffer[1] |= 127;
            *reinterpret_cast<uint64_t*>(&packetBuffer[headerPosition]) = htobe64(static_cast<uint64_t>(lengthToSend));
            headerPosition += 8;
        }

        if (mask) {
            *reinterpret_cast<uint32_t*>(&packetBuffer[headerPosition]) = htobe32(static_cast<uint32_t>(key));
            headerPosition += 4;

            for (size_t i = 0, o = 0; i < lengthToSend; i++, o = i % 4) {
                uint8_t shift = (3 - o) << 3;
                packetBuffer[i + headerPosition] = data_u8[i + bufferPosition] ^ ((shift == 0 ? key : (key >> shift)) & 0xFF);
            }
        } else {
            memcpy(packetBuffer + headerPosition, data_u8 + bufferPosition, lengthToSend);
        }
        
        try {
            mClient->send(packetBuffer, lengthToSend + headerPosition);
        } catch (std::runtime_error& e) {
            std:: cerr << "WebSocket send failed (" << e.what() << ")" << std::endl;
            onDisconnect();
            mClient->mErrorFlag = true;
            goto cleanup;
        }

        bufferPosition += lengthToSend;
    } while (bufferPosition < length);

    cleanup:
    delete[] packetBuffer;
}

void WebsockClientHandler::sendDisconnect() {
    sendRaw(WSOPC_DISCONNECT, nullptr, 0);
}

void WebsockClientHandler::sendText(const std::string& str) {
    sendRaw(WSOPC_TEXT, str.data(), str.size());
}

void WebsockClientHandler::sendBinary(const void* data, size_t length) {
    sendRaw(WSOPC_BINARY, data, length);
}

#endif