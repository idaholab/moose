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
#include "MooseTypes.h"

// libMesh
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/enum_fe_family.h"

// Forward declarations
class MultiMooseEnum;
namespace libMesh
{
class Point;
}

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

template <>
RelationshipManagerType stringToEnum<RelationshipManagerType>(const std::string & s);

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

// overloads for integer types where std::to_string gives the same result and is faster
inline std::string
stringify(int v)
{
  return std::to_string(v);
}
inline std::string
stringify(long v)
{
  return std::to_string(v);
}
inline std::string
stringify(long long v)
{
  return std::to_string(v);
}
inline std::string
stringify(unsigned int v)
{
  return std::to_string(v);
}
inline std::string
stringify(unsigned long v)
{
  return std::to_string(v);
}
inline std::string
stringify(unsigned long long v)
{
  return std::to_string(v);
}

/// Convert solve type into human readable string
std::string stringify(const SolveType & t);

/// Convert eigen solve type into human readable string
std::string stringify(const EigenSolveType & t);

/// Convert variable field type into human readable string
std::string stringify(const VarFieldType & t);

/// Add no-op stringify if the argument already is a string (must use overloading)
std::string stringify(const std::string & s);

/// Convert FEType from libMesh into string
std::string stringify(FEFamily f);

/// Convert SolutionIterationType into string
std::string stringify(SolutionIterationType t);

/// Add pair stringify to support maps
template <typename T, typename U>
std::string
stringify(const std::pair<T, U> & p, const std::string & delim = ":")
{
  return stringify(p.first) + delim + stringify(p.second);
}

/**
 * Convert a container to a string with elements separated by delimiter of user's choice
 *
 * Optionally, the container elements can be enclosed by curly braces and can
 * enclose elements in quotations (or other characters) to make the separation
 * of elements more clear.
 *
 * @param[in] c           Container to stringify
 * @param[in] delim       String to print between elements
 * @param[in] elem_encl   String to use at the beginning and end of each element,
 *                        typically quotation marks
 * @param[in] enclose_list_in_curly_braces   Enclose the list string in curly braces?
 */
template <template <typename...> class T, typename... U>
std::string
stringify(const T<U...> & c,
          const std::string & delim = ", ",
          const std::string & elem_encl = "",
          bool enclose_list_in_curly_braces = false)
{
  std::string str;
  if (enclose_list_in_curly_braces)
    str += "{";
  const auto begin = c.begin();
  const auto end = c.end();
  for (auto i = begin; i != end; ++i)
    str += (i != begin ? delim : "") + elem_encl + stringify(*i) + elem_encl;
  if (enclose_list_in_curly_braces)
    str += "}";
  return str;
}

/**
 * Stringify Reals with enough precision to guarantee lossless
 * Real -> string -> Real roundtrips.
 */
std::string stringifyExact(Real);
}

template <typename T1, typename T2>
std::vector<T2>
vectorCast(std::vector<T2> in)
{
  std::vector<T2> out(in.begin(), in.end());
  return out;
}

/**
 * Convert point represented as std::vector into Point
 * @param pos Point represented as a vector
 * @return Converted point
 */
Point toPoint(const std::vector<Real> & pos);
