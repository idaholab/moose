// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

// Module includes
#include "FEBoundaryValueUserObject.h"

template <>
InputParameters
validParams<FEBoundaryValueUserObject>()
{
  InputParameters params = validParams<FEBoundaryBaseUserObject>();

  params.addClassDescription("Generates an FE representation for a boundary value condition using "
                             "a 'FunctionSeries'-type Function");

  return params;
}

FEBoundaryValueUserObject::FEBoundaryValueUserObject(const InputParameters & parameters)
  : FEBoundaryBaseUserObject(parameters)
{
  // Nothing here
}
