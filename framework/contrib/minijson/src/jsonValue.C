#include "jsonValue.h"
#include "jsonException.h"

namespace miniJson
{

// type interface
JsonType
JsonValue::getType() const noexcept
{
  if (std::holds_alternative<std::nullptr_t>(_val))
  {
    return JsonType::kNull;
  }
  else if (std::holds_alternative<bool>(_val))
  {
    return JsonType::kBool;
  }
  else if (std::holds_alternative<double>(_val))
  {
    return JsonType::kNumber;
  }
  else if (std::holds_alternative<std::string>(_val))
  {
    return JsonType::kString;
  }
  else if (std::holds_alternative<Json::_array>(_val))
  {
    return JsonType::kArray;
  }
  else
  {
    return JsonType::kObject;
  }
}

// interface for array and object
size_t
JsonValue::size() const
{
  if (std::holds_alternative<Json::_array>(_val))
  {
    return std::get<Json::_array>(_val).size();
  }
  else if (std::holds_alternative<Json::_object>(_val))
  {
    return std::get<Json::_object>(_val).size();
  }
  else
  {
    throw JsonException("not a array or object");
  }
}

// operator[] for array:random_access
const Json &
JsonValue::operator[](size_t pos) const
{
  if (std::holds_alternative<Json::_array>(_val))
  {
    return std::get<Json::_array>(_val)[pos];
  }
  else
  {
    throw JsonException("not a array");
  }
}

Json &
JsonValue::operator[](size_t pos)
{
  return const_cast<Json &>(static_cast<const JsonValue &>(*this)[pos]);
}

// operator[] for object:O(1) search
const Json &
JsonValue::operator[](const std::string & key) const
{
  if (std::holds_alternative<Json::_object>(_val))
  {
    return std::get<Json::_object>(_val).at(key);
  }
  else
  {
    throw JsonException("not a object");
  }
}

Json &
JsonValue::operator[](const std::string & key)
{
  return const_cast<Json &>(static_cast<const JsonValue &>(*this)[key]);
}

// convert interface
std::nullptr_t
JsonValue::toNull() const
{
  try
  {
    return std::get<std::nullptr_t>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a null");
  }
}

bool
JsonValue::toBool() const
{
  try
  {
    return std::get<bool>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a bool");
  }
}

double
JsonValue::toDouble() const
{
  try
  {
    return std::get<double>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a double");
  }
}

const std::string &
JsonValue::toString() const
{
  try
  {
    return std::get<std::string>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a string");
  }
}

const Json::_array &
JsonValue::toArray() const
{
  try
  {
    return std::get<Json::_array>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a array");
  }
}

const Json::_object &
JsonValue::toObject() const
{
  try
  {
    return std::get<Json::_object>(_val);
  }
  catch (const std::bad_variant_access &)
  {
    throw JsonException("not a object");
  }
}

} // namespace miniJson