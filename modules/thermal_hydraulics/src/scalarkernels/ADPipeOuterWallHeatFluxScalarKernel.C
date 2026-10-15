//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeOuterWallHeatFluxScalarKernel.h"

#include "Function.h"
#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallHeatFluxScalarKernel);

InputParameters
ADPipeOuterWallHeatFluxScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall, with a "
      "prescribed heat flux applied to its non-radial side.");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addRequiredParam<FunctionName>("heat_flux", "Applied heat flux function [W/m^2]");
  params.addRequiredParam<FunctionName>(
      "heated_perimeter", "Perimeter of this wall node over which the heat flux is applied [m]");

  return params;
}

ADPipeOuterWallHeatFluxScalarKernel::ADPipeOuterWallHeatFluxScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _Tin(ScalarCoupleable::adCoupledScalarValue("inner_wall_temperature")),
    _heat_flux(getFunction("heat_flux")),
    _heated_perimeter(getFunction("heated_perimeter"))
{
}

ADReal
ADPipeOuterWallHeatFluxScalarKernel::convectiveLinearHeatRate() const
{
  // Prescribed heat flux applied to this node, positive into the wall
  return _heat_flux.value(Base::_t, Point()) * _heated_perimeter.value(Base::_t, Point());
}

Real
ADPipeOuterWallHeatFluxScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
