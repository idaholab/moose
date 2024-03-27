#pragma once

#include <stdexcept>

namespace miniJson
{

class JsonException : public std::runtime_error
{
public: // ctor
  explicit JsonException(const std::string & errMsg) : runtime_error(errMsg) {}

public: // interface
  // what():Indication of exception information
  const char * what() const noexcept override { return runtime_error::what(); }
};

} // namespace miniJson
