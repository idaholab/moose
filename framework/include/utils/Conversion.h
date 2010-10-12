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

#include "Moose.h"

namespace Moose {

  template<typename T>
  T stringToEnum(const std::string & s);

  template<>
  TimeSteppingScheme stringToEnum<TimeSteppingScheme>(const std::string & s);

}

#endif //CONVERSION_H
