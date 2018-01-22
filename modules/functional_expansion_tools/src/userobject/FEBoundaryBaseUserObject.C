// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

// Module includes
#include "FEBoundaryBaseUserObject.h"

template <>
InputParameters
validParams<FEBoundaryBaseUserObject>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();

  params += validParams<FEIntegralBaseUserObjectParameters>();

  return params;
}

FEBoundaryBaseUserObject::FEBoundaryBaseUserObject(const InputParameters & parameters)
  : FEIntegralBaseUserObject(parameters)
{
  mooseInfo("Using FEInterface-type UserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FEBoundaryBaseUserObject::getCentroid() const
{
  return _current_side_elem->centroid();
}

Real
FEBoundaryBaseUserObject::getVolume() const
{
  return _current_side_volume;
}
