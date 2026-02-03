//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
  {
    static_assert(std::is_pointer<remove_cvref_t<P>>::value,
                  "AnyPointer can only be constructed from a raw pointer type");

    _ptr = static_cast<void *>(p);
    _type_id = typeid(typename std::remove_pointer<remove_cvref_t<P>>::type);
  }

  /// nullptr for backwards compatibility
  AnyPointer(std::nullptr_t) noexcept : _ptr(nullptr), _type_id(typeid(void)) {}

  AnyPointer() = default;
  AnyPtr(const AnyPtr &) = default;
  AnyPointer(AnyPtr &&) noexcept = default;
  AnyPointer & operator=(const AnyPtr &) = default;
  AnyPointer & operator=(AnyPtr &&) noexcept = default;

  // Nested exception type
  class bad_cast : public std::bad_cast
  {
  public:
    bad_cast(std::type_index requested, std::type_index stored, bool null_pointer)
      : msg_(make_msg(requested, stored, null_pointer))
    {
    }

    const char * what() const noexcept override { return _msg.c_str(); }

  private:
    std::string _msg;

    static std::string
    make_msg(std::type_index requested, std::type_index stored, bool null_pointer)
    {
      std::ostringstream oss;
      oss << "AnyPtr::bad_cast: requested '" << libMesh::demangle(requested.name());
      if (null_pointer)
        oss << " (stored pointer is nullptr)";
      else
        oss << "' but stored '" << libMesh::demangle(stored.name()) << "'";
      return oss.str();
    }
  };

  /// non-const get: allow retrieval of the exact type (with the original constness)
  template <class T>
  T * get_if() noexcept
  {
    if (type == typeid(T))
      return static_cast<T *>(ptr);
    return nullptr;
  }

  /// const get: returns const T* when stored type is T or const T
  template <class T>
  const T * get_if() const noexcept
  {
    // Allow stored = T          -> const T* (widening)
    // Allow stored = const T    -> const T*
    if (type == typeid(T) || type == typeid(std::add_const_t<T>))
      return static_cast<const T *>(ptr);
    return nullptr;
  }

  template <class T>
  T & get()
  {
    auto * ptr = get_if<T>();
    if (ptr)
      return *ptr;
    throw bad_cast(typeid(T), _type_id, _ptr == nullptr);
  }

  template <class T>
  const T & get() const
  {
    auto * ptr = get_if<T>();
    if (ptr)
      return *ptr;
    throw bad_cast(typeid(T), _type_id, _ptr == nullptr);
  }

private:
  std::type_index _type_id = typeinfo(void);
  void * _ptr = nullptr;
}

} // namespace Moose
