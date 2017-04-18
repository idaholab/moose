/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
ExecFlagType stringToEnum<ExecFlagType>(const std::string & s);

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

// Vector conversions
template <typename T>
std::vector<T> vectorStringsToEnum(const MultiMooseEnum & v);

template <>
std::vector<ExecFlagType> vectorStringsToEnum<ExecFlagType>(const MultiMooseEnum & v);

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
template <>
std::string stringify(const SolveType & t);

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
