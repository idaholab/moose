#pragma once

#include <variant>
#include "minijson.h"
#include "jsonException.h"

namespace miniJson
{

// Implement class:JsonValue
class JsonValue
{
public: // ctor
  explicit JsonValue(std::nullptr_t) : _val(nullptr) {}
  explicit JsonValue(bool val) : _val(val) {}
  explicit JsonValue(double val) : _val(val) {}
  explicit JsonValue(const std::string & val) : _val(val) {}
  explicit JsonValue(const Json::_array & val) : _val(val) {}
  explicit JsonValue(const Json::_object & val) : _val(val) {}

public: // move ctor for string,array and object
  explicit JsonValue(std::string && val) : _val(std::move(val)) {}
  explicit JsonValue(Json::_array && val) : _val(std::move(val)) {}
  explicit JsonValue(Json::_object && val) : _val(std::move(val)) {}

public: // dtor
  ~JsonValue() = default;

public: // type interface
  JsonType getType() const noexcept;

public: // interface for array and object
  size_t size() const;
  // random_access
  const Json & operator[](size_t) const;
  Json & operator[](size_t);
  // O(1) search(not insert)
  const Json & operator[](const std::string &) const;
  Json & operator[](const std::string &);

public: // convert jsonValue into value instance
  std::nullptr_t toNull() const;
  bool toBool() const;
  double toDouble() const;
  const std::string & toString() const;
  const Json::_array & toArray() const;
  const Json::_object & toObject() const;

private:
  // std::varient is a C++17 features,like union in C language
  // More information can be seen in
  // https://en.cppreference.com/w/cpp/utility/variant
  std::variant<std::nullptr_t, bool, double, std::string, Json::_array, Json::_object> _val;
};

} // namespace miniJson