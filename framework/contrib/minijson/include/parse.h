#pragma once

#include "minijson.h"
#include "jsonException.h"

namespace miniJson
{

// constexpr function to determine a char is a specified number
constexpr bool
is1to9(char ch)
{
  return ch >= '1' && ch <= '9';
}
constexpr bool
is0to9(char ch)
{
  return ch >= '0' && ch <= '9';
}

class Parser
{
public: // ctor
  explicit Parser(const char * cstr) noexcept : _start(cstr), _curr(cstr) {}
  explicit Parser(const std::string & content) noexcept
    : _start(content.c_str()), _curr(content.c_str())
  {
  }

public: // uncopyable
  Parser(const Parser &) = delete;
  Parser & operator=(const Parser &) = delete;

private: // parse_aux interface
  void parseWhitespace() noexcept;
  unsigned parse4hex();
  std::string encodeUTF8(unsigned u) noexcept;
  std::string parseRawString();
  // indicate the error pos
  void error(const std::string & msg) const;

private: // parse interface
  Json parseValue();
  Json parseLiteral(const std::string & literal);
  Json parseNumber();
  Json parseString();
  Json parseArray();
  Json parseObject();

public: // only public parse interface
  Json parse();

private: // private data member
  // two const char* points to the valid context's cur pos and start pos
  const char * _start;
  const char * _curr;
};
}