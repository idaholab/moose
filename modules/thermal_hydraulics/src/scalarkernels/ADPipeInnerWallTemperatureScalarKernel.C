//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeInnerWallTemperatureScalarKernel.h"

#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeInnerWallTemperatureScalarKernel);

InputParameters
ADPipeInnerWallTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription("Assembles transport equation for the temperature of the "
                             "inner-surface radial node of a pipe wall.");
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate in component. Takes a "
                       "scalar variable name");
  params.addCoupledVar("outer_wall_temperature",
                       {},
                       "Temperature of the outer-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addCoupledVar("fluid_temperature",
                       {},
                       "Fluid temperature adjacent to this wall node. Takes a scalar variable "
                       "name");
  params.addCoupledVar(
      "upstream_fluid_temperature",
      {},
      "Fluid temperature adjacent to the upstream wall node. Takes a scalar variable "
      "name");
  params.addCoupledVar(
      "downstream_fluid_temperature",
      {},
      "Fluid temperature adjacent to the downstream wall node. Takes a scalar variable "
      "name");
  params.addRequiredParam<MooseFunctorName>(
      "wetted_perimeter", "Wetted perimeter of the fluid flow channel adjacent to this node [m]");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Heat transfer coefficient of the fluid [W/(m^2-K)]");
  params.addParam<bool>(
      "use_fluid_node_temp",
      true,
      "Whether to use fluid node temperature for wall heat transfer or a segment-averaged value.");

  return params;
}

ADPipeInnerWallTemperatureScalarKernel::ADPipeInnerWallTemperatureScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tout(ScalarCoupleable::adCoupledScalarValue("outer_wall_temperature")),
    _Tf(ScalarCoupleable::adCoupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::adCoupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::adCoupledScalarValue("downstream_fluid_temperature")),
    _wetted_perimeter(this->template getFunctor<ADReal>("wetted_perimeter")),
    _h(this->template getFunctor<ADReal>("heat_transfer_coefficient"))
{
}

ADReal
ADPipeInnerWallTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  const Moose::PointArg arg = {Point(1, 2, 3)};
  const int i = 0;
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
  // Upwind the segment inlet temperature on the flow direction
  const auto Tin = 1.0 / 2.0 * (1 - _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tfdown[i] +
                   1.0 / 2.0 * (1 + _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tfup[i];

  // Bulk fluid temperature seen by the wall: segment average or fluid node temperature
  const bool node_bulk = getParam<bool>("use_fluid_node_temp");
  const auto Tbulk = node_bulk ? ADReal(_Tf[i]) : ADReal((_Tf[i] + Tin) / 2.0);

  // Convective heat transfer with the fluid
  return _h(arg, state) * _wetted_perimeter(arg, state) * (Tbulk - Base::_u[i]);
}

Real
ADPipeInnerWallTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
