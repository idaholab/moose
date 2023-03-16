//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXBoundaryBaseUserObject.h"

InputParameters
FXBoundaryBaseUserObject::validParams()
{
  InputParameters params = FXIntegralBaseUserObject<SideIntegralVariableUserObject>::validParams();
  return params;
}

FXBoundaryBaseUserObject::FXBoundaryBaseUserObject(const InputParameters & parameters)
  : FXIntegralBaseUserObject<SideIntegralVariableUserObject>(parameters)
{
  mooseInfo("Using FXInterface-type UserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FXBoundaryBaseUserObject::getCentroid() const
{
  return _current_side_elem->vertex_average();
}

Real
FXBoundaryBaseUserObject::getVolume() const
{
  return _current_side_volume;
}
