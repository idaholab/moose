#include "parse.h"
#include <cassert>   // assert
#include <cmath>     // Huge_Val
#include <cstdlib>   // strtod
#include <cstring>   // strncmp
#include <stdexcept> // runtime_error

namespace miniJson
{

// skip all whitespace
void
Parser::parseWhitespace() noexcept
{
  while (*_curr == ' ' || *_curr == '\t' || *_curr == '\r' || *_curr == '\n')
  {
    ++_curr;
  }
  _start = _curr;
}

unsigned
Parser::parse4hex()
{
  unsigned u = 0;
  for (int i = 0; i != 4; ++i)
  {
    auto ch = static_cast<unsigned>(toupper(*++_curr));
    u <<= 4;
    if (ch >= '0' && ch <= '9')
    {
      u |= (ch - '0');
    }
    else if (ch >= 'A' && ch <= 'F')
    {
      u |= ch - 'A' + 10;
    }
    else
    {
      error("INVALID UNICODE HEX");
    }
  }
  return u;
}

std::string
Parser::encodeUTF8(unsigned u) noexcept
{
  std::string utf8;
  if (u <= 0x7F)
  { // 0111,1111
    utf8.push_back(static_cast<char>(u & 0xff));
  }
  else if (u <= 0x7FF)
  {
    utf8.push_back(static_cast<char>(0xc0 | ((u >> 6) & 0xff)));
    utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
  }
  else if (u <= 0xFFFF)
  {
    utf8.push_back(static_cast<char>(0xe0 | ((u >> 12) & 0xff)));
    utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
    utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
  }
  else
  {
    assert(u <= 0x10FFFF);
    utf8.push_back(static_cast<char>(0xf0 | ((u >> 18) & 0xff)));
    utf8.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3f)));
    utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
    utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
  }
  return utf8;
}

std::string
Parser::parseRawString()
{
  std::string str;
  while (true)
  {
    switch (*++_curr)
    {
      case '\"':
        _start = ++_curr;
        return str;
      case '\0':
        error("MISS QUOTATION MARK");
        break;
        return std::string();
      default:
        if (static_cast<unsigned char>(*_curr) < 0x20)
          error("INVALID STRING CHAR");
        str.push_back(*_curr);
        break;
      case '\\':
        switch (*++_curr)
        {
          case '\"':
            str.push_back('\"');
            break;
          case '\\':
            str.push_back('\\');
            break;
          case '/':
            str.push_back('/');
            break;
          case 'b':
            str.push_back('\b');
            break;
          case 'f':
            str.push_back('\f');
            break;
          case 'n':
            str.push_back('\n');
            break;
          case 't':
            str.push_back('\t');
            break;
          case 'r':
            str.push_back('\r');
            break;
          case 'u':
          {
            unsigned u1 = parse4hex();
            if (u1 >= 0xd800 && u1 <= 0xdbff)
            { // high surrogate
              if (*++_curr != '\\')
                error("INVALID UNICODE SURROGATE");
              if (*++_curr != 'u')
                error("INVALID UNICODE SURROGATE");
              unsigned u2 = parse4hex(); // low surrogate
              if (u2 < 0xdc00 || u2 > 0xdfff)
                error("INVALID UNICODE SURROGATE");
              u1 = (((u1 - 0xd800) << 10) | (u2 - 0xdc00)) + 0x10000;
            }
            str += encodeUTF8(u1);
          }
          break;
          default:
            error("INVALID STRING ESCAPE");
        }
        break;
    }
  }
}

void
Parser::error(const std::string & msg) const
{
  throw JsonException(msg + ": " + _start);
}

Json
Parser::parseValue()
{
  switch (*_curr)
  {
    case 'n':
      return parseLiteral("null");
    case 't':
      return parseLiteral("true");
    case 'f':
      return parseLiteral("false");
    case '\"':
      return parseString();
    case '[':
      return parseArray();
    case '{':
      return parseObject();
    case '\0':
      error("EXPECT VALUE");
      return parseLiteral("null");
    default:
      return parseNumber();
  }
}

Json
Parser::parseLiteral(const std::string & literal)
{
  // try to parse null && true && false
  if (strncmp(_curr, literal.c_str(), literal.size()) != 0)
    error("INVALID VALUE");
  _curr += literal.size();
  _start = _curr;
  switch (literal[0])
  {
    case 't':
      return Json(true);
    case 'f':
      return Json(false);
    default:
      return Json(nullptr);
  }
}

Json
Parser::parseNumber()
{
  if (*_curr == '-')
    ++_curr;
  if (*_curr == '0')
    ++_curr;
  else
  {
    if (!is1to9(*_curr))
      error("INVALID VALUE");
    while (is0to9(*++_curr))
      ; // pass all number character
  }
  if (*_curr == '.')
  {
    if (!is0to9(*++_curr)) // there must be a number character after '.'
      error("INVALID VALUE");
    while (is0to9(*++_curr))
      ;
  }
  if (toupper(*_curr) == 'E')
  {
    ++_curr;
    if (*_curr == '-' || *_curr == '+')
      ++_curr;
    if (!is0to9(*_curr))
      error("INVALID VALUE");
    while (is0to9(*++_curr))
      ;
  }
  // When we make sure that the current text is a number,
  // call the library function strtod
  double val = strtod(_start, nullptr);
  if (fabs(val) == HUGE_VAL)
    error("NUMBER TOO BIG");
  _start = _curr;
  return Json(val);
}

Json
Parser::parseString()
{
  return Json(parseRawString());
}

Json
Parser::parseArray()
{
  Json::_array arr;
  ++_curr; // skip '['
  parseWhitespace();
  if (*_curr == ']')
  {
    _start = ++_curr;
    return Json(arr);
  }
  while (true)
  {
    parseWhitespace();
    arr.push_back(parseValue()); // recursive
    parseWhitespace();
    if (*_curr == ',')
      ++_curr;
    else if (*_curr == ']')
    {
      _start = ++_curr;
      return Json(arr);
    }
    else
      error("MISS COMMA OR SQUARE BRACKET");
  }
}

Json
Parser::parseObject()
{
  Json::_object obj;
  ++_curr;
  parseWhitespace();
  if (*_curr == '}')
  {
    _start = ++_curr;
    return Json(obj);
  }
  while (true)
  {
    parseWhitespace();
    if (*_curr != '"')
      error("MISS KEY");
    std::string key = parseRawString();
    parseWhitespace();
    if (*_curr++ != ':')
      error("MISS COLON");
    parseWhitespace();
    Json val = parseValue();
    obj.insert({key, val});
    parseWhitespace();
    if (*_curr == ',')
      ++_curr;
    else if (*_curr == '}')
    {
      _start = ++_curr;
      return Json(obj);
    }
    else
      error("MISS COMMA OR CURLY BRACKET");
  }
}

Json
Parser::parse()
{
  // JSON-text = ws value ws
  // ws = *(%x20 / %x09 / %x0A / %x0D)
  parseWhitespace();
  Json json = parseValue();
  parseWhitespace();
  if (*_curr)
    // some character still exists after the end whitespace
    error("ROOT NOT SINGULAR");
  return json;
}

} // namespace miniJson