//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONVERSION_H
#define CONVERSION_H

// MOOSE includes
#include "MooseTypes.h"

// libMesh
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/point.h"

// Forward declarations
class MultiMooseEnum;

namespace Moose
{
// Scalar conversions
template <typename T>
T stringToEnum(const std::string & s);

template <>
QuadratureType stringToEnum<QuadratureType>(const std::string & s);

template <>
Order stringToEnum<Order>(const std::string & s);

template <>
CoordinateSystemType stringToEnum<CoordinateSystemType>(const std::string & s);

template <>
SolveType stringToEnum<SolveType>(const std::string & s);

template <>
LineSearchType stringToEnum<LineSearchType>(const std::string & s);

template <>
TimeIntegratorType stringToEnum<TimeIntegratorType>(const std::string & s);

// Vector conversions
template <typename T>
std::vector<T> vectorStringsToEnum(const MultiMooseEnum & v);

/// conversion to string
template <typename T>
std::string
stringify(const T & t)
{
  std::ostringstream os;
  os << t;
  return os.str();
}

/// Convert solve type into human readable string
std::string stringify(const SolveType & t);

/// Add no-op stringify if the argument already is a string (must use overloading)
std::string stringify(const std::string & s);

/// Add pair stringify to support maps
template <typename T, typename U>
std::string
stringify(const std::pair<T, U> & p)
{
  return stringify(p.first) + ':' + stringify(p.second);
}

/// Convert a container to a flat comma (or otherwise) separated string
template <template <typename...> class T, typename... U>
std::string
stringify(const T<U...> & c, const std::string & delim = ",")
{
  std::string str;
  const auto begin = c.begin(), end = c.end();
  for (auto i = begin; i != end; ++i)
    str += (i != begin ? delim : "") + stringify(*i);
  return str;
}

/**
 * Stringify Reals with enough precision to guarantee lossless
 * Real -> string -> Real roundtrips.
 */
std::string stringifyExact(Real);
}

/**
 * Convert point represented as std::vector into Point
 * @param pos Point represented as a vector
 * @return Converted point
 */
Point toPoint(const std::vector<Real> & pos);

#endif // CONVERSION_H
