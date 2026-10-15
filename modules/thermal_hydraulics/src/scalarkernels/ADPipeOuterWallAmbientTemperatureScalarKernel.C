//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeOuterWallAmbientTemperatureScalarKernel.h"

#include "Function.h"
#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallAmbientTemperatureScalarKernel);

InputParameters
ADPipeOuterWallAmbientTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall.");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function [K]");
  params.addRequiredParam<FunctionName>("htc_ambient",
                                        "Ambient heat transfer coefficient function [W/(m^2*K)]");
  params.addRequiredParam<FunctionName>(
      "ambient_perimeter", "Perimeter of this wall node exposed to the ambient environment [m]");

  return params;
}

ADPipeOuterWallAmbientTemperatureScalarKernel::ADPipeOuterWallAmbientTemperatureScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _Tin(ScalarCoupleable::adCoupledScalarValue("inner_wall_temperature")),
    _T_ambient(getFunction("T_ambient")),
    _htc_ambient(getFunction("htc_ambient")),
    _ambient_perimeter(getFunction("ambient_perimeter"))
{
}

ADReal
ADPipeOuterWallAmbientTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  // Convective heat transfer with the ambient environment
  return _htc_ambient.value(Base::_t, Point()) * _ambient_perimeter.value(Base::_t, Point()) *
         (_T_ambient.value(Base::_t, Point()) - Base::_u[0]);
}

Real
ADPipeOuterWallAmbientTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
