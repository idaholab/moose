//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeOuterWallAmbientTemperatureScalarKernel.h"

#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallAmbientTemperatureScalarKernel);

InputParameters
ADPipeOuterWallAmbientTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription("Assembles transport equation for the temperature of the "
                             "outer-surface radial node of a pipe wall, "
                             "with heat transfer to an ambient environment.");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>(
      "htc_ambient", "Ambient heat transfer coefficient functor [W/(m^2*K)]");
  params.addRequiredParam<MooseFunctorName>(
      "ambient_perimeter", "Perimeter of this wall node exposed to the ambient environment [m]");

  return params;
}

ADPipeOuterWallAmbientTemperatureScalarKernel::ADPipeOuterWallAmbientTemperatureScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _Tin(ScalarCoupleable::adCoupledScalarValue("inner_wall_temperature")),
    _T_ambient(this->template getFunctor<ADReal>("T_ambient")),
    _htc_ambient(this->template getFunctor<ADReal>("htc_ambient")),
    _ambient_perimeter(this->template getFunctor<ADReal>("ambient_perimeter"))
{
}

ADReal
ADPipeOuterWallAmbientTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  const Moose::PointArg arg = {Point(1, 2, 3)};
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();

  // Convective heat transfer with the ambient environment
  return _htc_ambient(arg, state) * _ambient_perimeter(arg, state) *
         (_T_ambient(arg, state) - Base::_u[0]);
}

Real
ADPipeOuterWallAmbientTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
