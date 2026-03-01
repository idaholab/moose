//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"

#include <sstream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <type_traits>

namespace Moose
{

class AnyPointer
{
  template <class T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

public:
  template <class P>
  explicit AnyPointer(P p) noexcept
    : _type_id(typeid(typename std::remove_pointer<remove_cvref_t<P>>::type)),
      _ptr(const_cast<void *>(static_cast<const void *>(p)))
  {
    static_assert(std::is_pointer<remove_cvref_t<P>>::value,
                  "AnyPointer can only be constructed from a raw pointer type");
  }

  /// nullptr for backwards compatibility
  AnyPointer(std::nullptr_t) noexcept : _type_id(typeid(void)), _ptr(nullptr) {}

  AnyPointer() = default;
  AnyPointer(const AnyPointer &) = default;
  AnyPointer(AnyPointer &&) noexcept = default;
  AnyPointer & operator=(const AnyPointer &) = default;
  AnyPointer & operator=(AnyPointer &&) noexcept = default;

  // Nested exception type
  class bad_cast : public std::bad_cast
  {
  public:
    bad_cast(std::type_index requested, std::type_index stored, bool null_pointer)
      : _msg(make_msg(requested, stored, null_pointer))
    {
    }

    const char * what() const noexcept override { return _msg.c_str(); }

  private:
    std::string _msg;

    static std::string
    make_msg(std::type_index requested, std::type_index stored, bool null_pointer)
    {
      std::ostringstream oss;
      oss << "AnyPointer::bad_cast: requested '" << libMesh::demangle(requested.name());
      if (null_pointer)
        oss << "' (stored pointer is nullptr)";
      else
        oss << "' but stored '" << libMesh::demangle(stored.name()) << "'";
      return oss.str();
    }
  };

  /// Check if the pointer has a value (is not default-constructed or null)
  bool hasValue() const noexcept { return _ptr != nullptr; }

  /// non-const get_if: allow retrieval of the exact type (with the original constness)
  template <class T>
  T * get_if() noexcept
  {
    if (_type_id == typeid(T))
      return static_cast<T *>(_ptr);
    return nullptr;
  }

  /// const get_if: returns const T* when stored type is T or const T
  template <class T>
  const T * get_if() const noexcept
  {
    // Allow stored = T          -> const T* (widening)
    // Allow stored = const T    -> const T*
    if (_type_id == typeid(T) || _type_id == typeid(std::add_const_t<T>))
      return static_cast<const T *>(_ptr);
    return nullptr;
  }

  template <class T>
  T & get()
  {
    auto * p = get_if<T>();
    if (p)
      return *p;
    throw bad_cast(typeid(T), _type_id, _ptr == nullptr);
  }

  template <class T>
  const T & get() const
  {
    auto * p = get_if<T>();
    if (p)
      return *p;
    throw bad_cast(typeid(T), _type_id, _ptr == nullptr);
  }

  /// Get the raw pointer without type checking. Used when context type is not needed
  /// (e.g., dataStore which ignores context for many types).
  void * getRawPtr() noexcept { return _ptr; }
  const void * getRawPtr() const noexcept { return _ptr; }

private:
  std::type_index _type_id = typeid(void);
  void * _ptr = nullptr;
};

} // namespace Moose
