//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseTypes.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

#include <tuple>

namespace Moose
{

/**
 * Serial access requires object data to be stored contiguously. Specialize this template
 * to support more types.
 */
template <typename T>
struct SerialAccess
{
  static_assert(always_false<T>, "Specialize SerialAccess for this type.");
};

// Specializations for scalar types
#define SERIAL_ACCESS_SCALAR(type)                                                                 \
  template <>                                                                                      \
  struct SerialAccess<type>                                                                        \
  {                                                                                                \
    static type * data(type & obj) { return &obj; }                                                \
    static constexpr std::size_t size(type &) { return 1u; }                                       \
    static constexpr std::size_t size() { return 1u; }                                             \
  }

SERIAL_ACCESS_SCALAR(Real);
SERIAL_ACCESS_SCALAR(const Real);
SERIAL_ACCESS_SCALAR(ADReal);
SERIAL_ACCESS_SCALAR(const ADReal);

// constant size containers
#define SERIAL_ACCESS_CONST_SIZE(type, dataptr, sizeval)                                           \
  template <typename T>                                                                            \
  struct SerialAccess<type<T>>                                                                     \
  {                                                                                                \
    static auto * data(type<T> & obj) { return dataptr; }                                          \
    static constexpr std::size_t size(type<T> &) { return sizeval; }                               \
    static constexpr std::size_t size() { return sizeval; }                                        \
  }

SERIAL_ACCESS_CONST_SIZE(libMesh::VectorValue, &obj(0u), Moose::dim);
SERIAL_ACCESS_CONST_SIZE(const libMesh::VectorValue, &obj(0u), Moose::dim);
SERIAL_ACCESS_CONST_SIZE(RankTwoTensorTempl, &obj(0u, 0u), RankTwoTensorTempl<T>::N2);
SERIAL_ACCESS_CONST_SIZE(const RankTwoTensorTempl, &obj(0u, 0u), RankTwoTensorTempl<T>::N2);
SERIAL_ACCESS_CONST_SIZE(RankFourTensorTempl, &obj(0u, 0u, 0u, 0u), RankFourTensorTempl<T>::N4);
SERIAL_ACCESS_CONST_SIZE(const RankFourTensorTempl,
                         &obj(0u, 0u, 0u, 0u),
                         RankFourTensorTempl<T>::N4);

// dynamic size containers (determining size requires an object instance)
#define SERIAL_ACCESS_DYNAMIC_SIZE(type, dataptr, sizeval)                                         \
  template <typename T>                                                                            \
  struct SerialAccess<type<T>>                                                                     \
  {                                                                                                \
    static auto * data(type<T> & obj) { return dataptr; }                                          \
    static constexpr std::size_t size(type<T> & obj) { return sizeval; }                           \
  }

SERIAL_ACCESS_DYNAMIC_SIZE(DenseVector, &obj(0u), obj.size());

/**
 * Value type helper (necessary for any type that does not have a value_type
 * member or where value_type doesn't have a suitable meaning (ADReal)).
 */
template <typename T>
struct SerialAccessValueTypeHelper
{
  typedef typename T::value_type value_type;
};
template <>
struct SerialAccessValueTypeHelper<ADReal>
{
  typedef ADReal value_type;
};
template <>
struct SerialAccessValueTypeHelper<Real>
{
  typedef Real value_type;
};

template <typename T>
class SerialAccessRange
{
public:
  /// Value type of the components of T
  typedef typename SerialAccessValueTypeHelper<typename std::remove_const<T>::type>::value_type R;
  /// Value type with the correct constness
  typedef typename std::conditional<std::is_const_v<T>, const R, R>::type V;

  class iterator
  {
  public:
    iterator(V * i) : _i(i) {}

    V & operator*() const { return *_i; }

    const iterator & operator++()
    {
      ++_i;
      return *this;
    }

    iterator operator++(int)
    {
      iterator returnval(*this);
      ++_i;
      return returnval;
    }

    bool operator==(const iterator & j) const { return (_i == j._i); }
    bool operator!=(const iterator & j) const { return !(*this == j); }

  private:
    V * _i;
  };

  SerialAccessRange(T & obj)
    : _begin(SerialAccess<T>::data(obj)),
      _end(SerialAccess<T>::data(obj) + SerialAccess<T>::size(obj))
  {
  }

  iterator begin() const { return _begin; }
  iterator end() const { return _end; }

  V & operator[](int i) { return *(&*_begin + i); }

private:
  iterator _begin, _end;
};

template <typename T>
SerialAccessRange<T>
serialAccess(T & obj)
{
  return SerialAccessRange<T>(obj);
}

/// Helper structure to hold a list of types
template <typename... Ts>
struct TypeList
{
  typedef std::tuple<Ts...> Tuple;
  typedef std::tuple<Ts *...> PointerTuple;
  static constexpr std::size_t size = sizeof...(Ts);
};

/// Type loop
template <template <typename, int> class L, int I, typename T, typename... Ts, typename... As>
void
typeLoopInternal(TypeList<T, Ts...>, As... args)
{
  L<T, I>::apply(args...);
  if constexpr (sizeof...(Ts) > 0)
    typeLoopInternal<L, I + 1>(TypeList<Ts...>{}, args...);
}

/// Type loop
template <template <typename, int> class L, typename... Ts, typename... As>
void
typeLoop(TypeList<Ts...>, As... args)
{
  typeLoopInternal<L, 0>(TypeList<Ts...>{}, args...);
}

} // namespace Moose;
