#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace miniJson
{

enum class JsonType
{
  kNull,
  kBool,
  kNumber,
  kString,
  kArray,
  kObject
};

// PIMPL
// forward declaration for std::unique_ptr
class JsonValue;

class Json final
{
public: // alias declarations
  using _array = std::vector<Json>;
  using _object = std::unordered_map<std::string, Json>;

public: // ctor for the various types of JSON value
  Json() : Json(nullptr){};
  Json(std::nullptr_t);
  Json(bool);
  // convert int into double
  Json(int val) : Json(1.0 * val) {}
  Json(double);
  // without this ctor, Json("xx") will call Json(bool)
  Json(const char * cstr) : Json(std::string(cstr)) {}
  Json(const std::string &);
  Json(std::string &&);
  Json(const _array &);
  Json(_array &&);
  Json(const _object &);
  Json(_object &&);

  // prevents Json(some_pointer) from accidentally producing a bool.
  Json(void *) = delete;

public: // implicit ctor for objects
  // Implicit constructor: map-like objects (std::map, std::unordered_map,
  // etc)
  template <
      class M,
      typename std::enable_if<
          std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value &&
              std::is_constructible<Json, decltype(std::declval<M>().begin()->second)>::value,
          int>::type = 0>
  Json(const M & m) : Json(_object(m.begin(), m.end()))
  {
  }
  // Implicit constructor: vector-like objects (std::list, std::vector,
  // std::set, etc)
  template <class V,
            typename std::enable_if<
                std::is_constructible<Json, decltype(*std::declval<V>().begin())>::value,
                int>::type = 0>
  Json(const V & v) : Json(_array(v.begin(), v.end()))
  {
  }

public: // dtor
  ~Json();

public:                                    // copy constructor && assignment
  Json(const Json &);                      // deeply copy
  Json & operator=(const Json &) noexcept; // copy && swap

public: // move constructor && assignment
  Json(Json &&) noexcept;
  Json & operator=(Json &&) noexcept;

public: // parse && serialize interface
  // errMsg can store exception message(all exception will be catched)
  static Json parse(const std::string & content, std::string & errMsg) noexcept;
  std::string serialize() const noexcept;

public: // type interface
  JsonType getType() const noexcept;
  bool isNull() const noexcept;
  bool isBool() const noexcept;
  bool isNumber() const noexcept;
  bool isString() const noexcept;
  bool isArray() const noexcept;
  bool isObject() const noexcept;

public: // convert json object into value
  bool toBool() const;
  double toDouble() const;
  const std::string & toString() const;
  const _array & toArray() const;
  const _object & toObject() const;

public: // interface for array && object
  size_t size() const;
  // operator[] for array
  Json & operator[](size_t);
  const Json & operator[](size_t) const;
  // operator[] for object
  Json & operator[](const std::string &);
  const Json & operator[](const std::string &) const;

private:                      // aux interface
  void swap(Json &) noexcept; // make copy && swap
  std::string serializeString() const noexcept;
  std::string serializeArray() const noexcept;
  std::string serializeObject() const noexcept;

private: // data member
  std::unique_ptr<JsonValue> _jsonValue;
};

// non-member function
inline std::ostream &
operator<<(std::ostream & os, const Json & json)
{
  return os << json.serialize();
}
bool operator==(const Json &, const Json &);
inline bool
operator!=(const Json & lhs, const Json & rhs)
{
  return !(lhs == rhs);
}

} // namespace miniJson
