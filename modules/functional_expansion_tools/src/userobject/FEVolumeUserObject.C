// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

// Module includes
#include "FEVolumeUserObject.h"

template <>
InputParameters
validParams<FEVolumeUserObject>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();

  params += validParams<FEIntegralBaseUserObjectParameters>();

  params.addClassDescription("Generates an FE representation of a variable value over a volume "
                             "using a 'FunctionSeries'-type Function");

  return params;
}

FEVolumeUserObject::FEVolumeUserObject(const InputParameters & parameters)
  : FEIntegralBaseUserObject<ElementIntegralVariableUserObject>(parameters)
{
  mooseInfo("Using FEVolumeUserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FEVolumeUserObject::getCentroid() const
{
  return _current_elem->centroid();
}

Real
FEVolumeUserObject::getVolume() const
{
  return _current_elem_volume;
}
