//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosReferenceWrapper.h"

#include <type_traits>
#include <utility>

#ifdef MOOSE_KOKKOS_SCOPE
#define KOKKOS_SCALAR_FUNCTION KOKKOS_FUNCTION
#else
#define KOKKOS_SCALAR_FUNCTION
#endif

namespace Moose::Kokkos
{

/**
 * The Kokkos wrapper class that can hold the reference of an arithmetic scalar variable
 */
template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
class Scalar : public ReferenceWrapper<T>
{
public:
  using value_type = T;

  /**
   * Constructor
   * @param value The writeable reference of the arithmetic scalar variable to store
   */
  Scalar(T & value) : ReferenceWrapper<T>(value) {}
  /**
   * Copy constructor
   */
  Scalar(const Scalar & object) : ReferenceWrapper<T>(object) {}

  /**
   * Assign a scalar value to the underlying host reference
   * @param value The scalar value to be assigned
   */
  auto & operator=(const Scalar & value)
  {
    this->_reference = static_cast<const T &>(value);

    return *this;
  }
  /**
   * Assign an arithmetic value to the underlying host reference
   * @param value The scalar value to be assigned
   */
  template <typename U>
  auto operator=(const U & value) -> decltype(std::declval<T &>() = value, std::declval<Scalar &>())
  {
    this->_reference = value;

    return *this;
  }

  /**
   * Get the positive value of the scalar
   * @returns The positive scalar value
   */
  KOKKOS_SCALAR_FUNCTION auto operator+() const -> decltype(+std::declval<const T &>())
  {
    return +value();
  }
  /**
   * Get the negated value of the scalar
   * @returns The negated scalar value
   */
  KOKKOS_SCALAR_FUNCTION auto operator-() const -> decltype(-std::declval<const T &>())
  {
    return -value();
  }

  /**
   * Add another value to the underlying host reference
   * @param value The value to add
   */
  template <typename U>
  auto operator+=(const U & value)
      -> decltype(std::declval<T &>() += value, std::declval<Scalar &>())
  {
    this->_reference += value;

    return *this;
  }
  /**
   * Subtract another value from the underlying host reference
   * @param value The value to subtract
   */
  template <typename U>
  auto operator-=(const U & value)
      -> decltype(std::declval<T &>() -= value, std::declval<Scalar &>())
  {
    this->_reference -= value;

    return *this;
  }
  /**
   * Multiply the underlying host reference by another value
   * @param value The value to multiply by
   */
  template <typename U>
  auto operator*=(const U & value)
      -> decltype(std::declval<T &>() *= value, std::declval<Scalar &>())
  {
    this->_reference *= value;

    return *this;
  }
  /**
   * Divide the underlying host reference by another value
   * @param value The value to divide by
   */
  template <typename U>
  auto operator/=(const U & value)
      -> decltype(std::declval<T &>() /= value, std::declval<Scalar &>())
  {
    this->_reference /= value;

    return *this;
  }
  /**
   * Assign the remainder after division by another value to the underlying host reference
   * @param value The divisor value
   */
  template <typename U>
  auto operator%=(const U & value)
      -> decltype(std::declval<T &>() %= value, std::declval<Scalar &>())
  {
    this->_reference %= value;

    return *this;
  }

  /**
   * Prefix increment the underlying host reference
   * @returns This scalar wrapper
   */
  template <typename U = T>
  auto operator++() -> decltype(++std::declval<U &>(), std::declval<Scalar &>())
  {
    ++this->_reference;

    return *this;
  }
  /**
   * Postfix increment the underlying host reference
   * @returns The previous scalar value
   */
  template <typename U = T>
  auto operator++(int) -> decltype(std::declval<U &>()++)
  {
    return this->_reference++;
  }
  /**
   * Prefix decrement the underlying host reference
   * @returns This scalar wrapper
   */
  template <typename U = T>
  auto operator--() -> decltype(--std::declval<U &>(), std::declval<Scalar &>())
  {
    --this->_reference;

    return *this;
  }
  /**
   * Postfix decrement the underlying host reference
   * @returns The previous scalar value
   */
  template <typename U = T>
  auto operator--(int) -> decltype(std::declval<U &>()--)
  {
    return this->_reference--;
  }

  /**
   * Add another value to this scalar
   * @param value The value to add
   * @returns The sum
   */
  template <typename U>
  KOKKOS_SCALAR_FUNCTION auto operator+(const U & value) const
      -> decltype(std::declval<const T &>() + value)
  {
    return this->value() + value;
  }
  /**
   * Subtract another value from this scalar
   * @param value The value to subtract
   * @returns The difference
   */
  template <typename U>
  KOKKOS_SCALAR_FUNCTION auto operator-(const U & value) const
      -> decltype(std::declval<const T &>() - value)
  {
    return this->value() - value;
  }
  /**
   * Multiply this scalar by another value
   * @param value The value to multiply by
   * @returns The product
   */
  template <typename U>
  KOKKOS_SCALAR_FUNCTION auto operator*(const U & value) const
      -> decltype(std::declval<const T &>() * value)
  {
    return this->value() * value;
  }
  /**
   * Divide this scalar by another value
   * @param value The value to divide by
   * @returns The quotient
   */
  template <typename U>
  KOKKOS_SCALAR_FUNCTION auto operator/(const U & value) const
      -> decltype(std::declval<const T &>() / value)
  {
    return this->value() / value;
  }
  /**
   * Get the remainder after division by another value
   * @param value The divisor value
   * @returns The remainder
   */
  template <typename U>
  KOKKOS_SCALAR_FUNCTION auto operator%(const U & value) const
      -> decltype(std::declval<const T &>() % value)
  {
    return this->value() % value;
  }

private:
  KOKKOS_SCALAR_FUNCTION const T & value() const { return static_cast<const T &>(*this); }
};

template <typename>
struct is_scalar : std::false_type
{
};

template <typename T, typename Enable>
struct is_scalar<Scalar<T, Enable>> : std::true_type
{
};

template <typename T,
          typename U,
          typename = typename std::enable_if<!is_scalar<typename std::decay<T>::type>::value>::type>
KOKKOS_SCALAR_FUNCTION auto
operator+(const T & left, const Scalar<U> & right) -> decltype(left + static_cast<const U &>(right))
{
  return left + static_cast<const U &>(right);
}

template <typename T,
          typename U,
          typename = typename std::enable_if<!is_scalar<typename std::decay<T>::type>::value>::type>
KOKKOS_SCALAR_FUNCTION auto
operator-(const T & left, const Scalar<U> & right) -> decltype(left - static_cast<const U &>(right))
{
  return left - static_cast<const U &>(right);
}

template <typename T,
          typename U,
          typename = typename std::enable_if<!is_scalar<typename std::decay<T>::type>::value>::type>
KOKKOS_SCALAR_FUNCTION auto
operator*(const T & left, const Scalar<U> & right) -> decltype(left * static_cast<const U &>(right))
{
  return left * static_cast<const U &>(right);
}

template <typename T,
          typename U,
          typename = typename std::enable_if<!is_scalar<typename std::decay<T>::type>::value>::type>
KOKKOS_SCALAR_FUNCTION auto
operator/(const T & left, const Scalar<U> & right) -> decltype(left / static_cast<const U &>(right))
{
  return left / static_cast<const U &>(right);
}

template <typename T,
          typename U,
          typename = typename std::enable_if<!is_scalar<typename std::decay<T>::type>::value>::type>
KOKKOS_SCALAR_FUNCTION auto
operator%(const T & left, const Scalar<U> & right) -> decltype(left % static_cast<const U &>(right))
{
  return left % static_cast<const U &>(right);
}

template <typename T>
struct ArrayDeepCopy<Scalar<T>>
{
  static constexpr bool value = true;
};

// Mimic MOOSE convention
using PostprocessorValue = Scalar<const PostprocessorValue>;

} // namespace Moose::Kokkos
