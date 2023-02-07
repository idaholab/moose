#include "json.h"
#include <cstdio>
#include "jsonValue.h"
#include "parse.h"

namespace miniJson
{

// Json's ctor && dtor
Json::Json(std::nullptr_t) : _jsonValue(std::make_unique<JsonValue>(nullptr)) {}
Json::Json(bool val) : _jsonValue(std::make_unique<JsonValue>(val)) {}
Json::Json(double val) : _jsonValue(std::make_unique<JsonValue>(val)) {}
Json::Json(const std::string & val) : _jsonValue(std::make_unique<JsonValue>(val)) {}
Json::Json(std::string && val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) {}
Json::Json(const _array & val) : _jsonValue(std::make_unique<JsonValue>(val)) {}
Json::Json(_array && val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) {}
Json::Json(const _object & val) : _jsonValue(std::make_unique<JsonValue>(val)) {}
Json::Json(_object && val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) {}

Json::~Json() = default;

// Json's copy constructor
Json::Json(const Json & rhs)
{
  switch (rhs.getType())
  {
    case JsonType::kNull:
    {
      _jsonValue = std::make_unique<JsonValue>(nullptr);
      break;
    }
    case JsonType::kBool:
    {
      _jsonValue = std::make_unique<JsonValue>(rhs.toBool());
      break;
    }
    case JsonType::kNumber:
    {
      _jsonValue = std::make_unique<JsonValue>(rhs.toDouble());
      break;
    }
    case JsonType::kString:
    {
      _jsonValue = std::make_unique<JsonValue>(rhs.toString());
      break;
    }
    case JsonType::kArray:
    {
      _jsonValue = std::make_unique<JsonValue>(rhs.toArray());
      break;
    }
    case JsonType::kObject:
    {
      _jsonValue = std::make_unique<JsonValue>(rhs.toObject());
      break;
    }
    default:
    {
      break;
    }
  }
}

// Json's copy assignment
Json &
Json::operator=(const Json & rhs) noexcept
{
  // copy && swap
  Json temp(rhs);
  swap(temp);
  return *this;
}

// Json's move operation=default
Json::Json(Json && rhs) noexcept = default;
Json & Json::operator=(Json && rhs) noexcept = default;

// parse interface(static member function)
Json
Json::parse(const std::string & content, std::string & errMsg) noexcept
{
  try
  {
    Parser p(content);
    return p.parse();
  }
  catch (JsonException & e)
  {
    errMsg = e.what();
    return Json(nullptr);
  }
}

std::string
Json::serialize() const noexcept
{
  switch (_jsonValue->getType())
  {
    case JsonType::kNull:
      return "null";
    case JsonType::kBool:
      return _jsonValue->toBool() ? "true" : "false";
    case JsonType::kNumber:
      char buf[32];
      snprintf(buf,
               sizeof(buf),
               "%.17g",
               _jsonValue->toDouble()); // enough to convert a double to a string
      return std::string(buf);
    case JsonType::kString:
      return serializeString();
    case JsonType::kArray:
      return serializeArray();
    default:
      return serializeObject();
  }
}

// type interface
JsonType
Json::getType() const noexcept
{
  return _jsonValue->getType();
}

bool
Json::isNull() const noexcept
{
  return getType() == JsonType::kNull;
}
bool
Json::isBool() const noexcept
{
  return getType() == JsonType::kBool;
}
bool
Json::isNumber() const noexcept
{
  return getType() == JsonType::kNumber;
}
bool
Json::isString() const noexcept
{
  return getType() == JsonType::kString;
}
bool
Json::isArray() const noexcept
{
  return getType() == JsonType::kArray;
}
bool
Json::isObject() const noexcept
{
  return getType() == JsonType::kObject;
}

// parse interface
bool
Json::toBool() const
{
  return _jsonValue->toBool();
}
double
Json::toDouble() const
{
  return _jsonValue->toDouble();
}
const std::string &
Json::toString() const
{
  return _jsonValue->toString();
}
const Json::_array &
Json::toArray() const
{
  return _jsonValue->toArray();
}
const Json::_object &
Json::toObject() const
{
  return _jsonValue->toObject();
}

// interface for array and object
size_t
Json::size() const
{
  return _jsonValue->size();
}
// operator[] for array
Json &
Json::operator[](size_t pos)
{
  return _jsonValue->operator[](pos);
}
const Json &
Json::operator[](size_t pos) const
{
  return _jsonValue->operator[](pos);
}
// operator[] for object
Json &
Json::operator[](const std::string & key)
{
  return _jsonValue->operator[](key);
}
const Json &
Json::operator[](const std::string & key) const
{
  return _jsonValue->operator[](key);
}

// aux interface for copy && swap
void
Json::swap(Json & rhs) noexcept
{
  using std::swap;
  swap(_jsonValue, rhs._jsonValue);
}

// aux interface for serialize
std::string
Json::serializeString() const noexcept
{
  std::string res = "\"";
  for (auto e : _jsonValue->toString())
  {
    switch (e)
    {
      case '\"':
        res += "\\\"";
        break;
      case '\\':
        res += "\\\\";
        break;
      case '\b':
        res += "\\b";
        break;
      case '\f':
        res += "\\f";
        break;
      case '\n':
        res += "\\n";
        break;
      case '\r':
        res += "\\r";
        break;
      case '\t':
        res += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(e) < 0x20)
        {
          char buf[7];
          snprintf(buf, 7, "\\u%04X", e);
          res += buf;
        }
        else
          res += e;
    }
  }
  return res + '"';
}

std::string
Json::serializeArray() const noexcept
{
  std::string res = "[ ";
  for (size_t i = 0; i != _jsonValue->size(); ++i)
  {
    if (i > 0)
    {
      res += ", ";
    }
    res += (*this)[i].serialize();
  }
  return res + " ]";
}

std::string
Json::serializeObject() const noexcept
{
  std::string res = "{ ";
  bool first = true; // indicate now is the first object
  for (auto && p : _jsonValue->toObject())
  {
    if (first)
    {
      first = false;
    }
    else
    {
      res += ", ";
    }
    res += "\"" + p.first + "\"";
    res += ": ";
    res += p.second.serialize();
  }
  return res + " }";
}

bool
operator==(const Json & lhs, const Json & rhs)
{
  if (lhs.getType() != rhs.getType())
  {
    return false;
  }
  switch (lhs.getType())
  {
    case JsonType::kNull:
    {
      return true;
    }
    case JsonType::kBool:
    {
      return lhs.toBool() == rhs.toBool();
    }
    case JsonType::kNumber:
    {
      return lhs.toDouble() == rhs.toDouble();
    }
    case JsonType::kString:
    {
      return lhs.toString() == rhs.toString();
    }
    case JsonType::kArray:
    {
      return lhs.toArray() == rhs.toArray();
    }
    case JsonType::kObject:
    {
      return lhs.toObject() == rhs.toObject();
    }
    default:
    {
      return false;
    }
  }
}

} // namespace miniJson
