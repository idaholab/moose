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

#include "ExecStore.h"
#include "MooseTypes.h"
// libMesh
#include "enum_order.h"
#include "enum_quadrature_type.h"
#include "point.h"

namespace Moose {

  template<typename T>
  T stringToEnum(const std::string & s);

  template<>
  TimeSteppingScheme stringToEnum<TimeSteppingScheme>(const std::string & s);

  template<>
  ExecFlagType stringToEnum<ExecFlagType>(const std::string & s);

  template<>
  QuadratureType stringToEnum<QuadratureType>(const std::string & s);

  template<>
  Order stringToEnum<Order>(const std::string & s);

  template<>
  CoordinateSystemType stringToEnum<CoordinateSystemType>(const std::string & s);

  template<>
  PPSOutputType stringToEnum<PPSOutputType>(const std::string & s);

  // conversion to string
  template<typename T>
  std::string
  stringify(const T & t)
  {
    std::ostringstream os;
    os << t;
    return os.str();
  }

}

/**
 * Convert point represented as std::vector into Point
 * @param pt Point represented as a vector
 * @return Converted point
 */
Point toPoint(const std::vector<Real> & pos);

#endif //CONVERSION_H
