//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXVolumeUserObject.h"

registerMooseObject("FunctionalExpansionToolsApp", FXVolumeUserObject);

InputParameters
FXVolumeUserObject::validParams()
{
  InputParameters params =
      FXIntegralBaseUserObject<ElementIntegralVariableUserObject>::validParams();

  params.addClassDescription("Generates an Functional Expansion representation of a variable value "
                             "over a volume using a 'FunctionSeries'-type Function");

  return params;
}

FXVolumeUserObject::FXVolumeUserObject(const InputParameters & parameters)
  : FXIntegralBaseUserObject<ElementIntegralVariableUserObject>(parameters)
{
  mooseInfo("Using FXVolumeUserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FXVolumeUserObject::getCentroid() const
{
  return _current_elem->vertex_average();
}

Real
FXVolumeUserObject::getVolume() const
{
  return _current_elem_volume;
}
