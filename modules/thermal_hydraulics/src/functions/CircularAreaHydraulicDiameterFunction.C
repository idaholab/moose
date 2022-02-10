//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircularAreaHydraulicDiameterFunction.h"

registerMooseObject("ThermalHydraulicsApp", CircularAreaHydraulicDiameterFunction);

InputParameters
CircularAreaHydraulicDiameterFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FunctionName>("area_function", "Area function");

  params.addClassDescription(
      "Computes hydraulic diameter for a circular area from its area function");

  return params;
}

CircularAreaHydraulicDiameterFunction::CircularAreaHydraulicDiameterFunction(
    const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _area_function(getFunction("area_function"))
{
}

Real
CircularAreaHydraulicDiameterFunction::value(Real t, const Point & p) const
{
  const Real A = _area_function.value(t, p);
  return std::sqrt(4.0 * A / M_PI);
}

RealVectorValue
CircularAreaHydraulicDiameterFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
