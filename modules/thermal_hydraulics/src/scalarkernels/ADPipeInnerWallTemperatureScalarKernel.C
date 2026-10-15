//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeInnerWallTemperatureScalarKernel.h"

#include "Function.h"
#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeInnerWallTemperatureScalarKernel);

InputParameters
ADPipeInnerWallTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params += DittusBoelterHTCInterface::validParams();
  params.addClassDescription(
      "Solves for the temperature of the inner-surface radial node of a pipe wall.");
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
  params.addRequiredParam<FunctionName>(
      "wetted_perimeter", "Wetted perimeter of the fluid flow channel adjacent to this node [m]");

  return params;
}

ADPipeInnerWallTemperatureScalarKernel::ADPipeInnerWallTemperatureScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    DittusBoelterHTCInterface(this),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tout(ScalarCoupleable::adCoupledScalarValue("outer_wall_temperature")),
    _Tf(ScalarCoupleable::adCoupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::adCoupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::adCoupledScalarValue("downstream_fluid_temperature")),
    _wetted_perimeter(getFunction("wetted_perimeter"))
{
}

Real
ADPipeInnerWallTemperatureScalarKernel::wettedPerimeter(const Real t) const
{
  return _wetted_perimeter.value(t, Point());
}

ADReal
ADPipeInnerWallTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  const int i = 0;
  // Time at which the correlation's pressure and flow area inputs are evaluated
  const Real t_props = Base::_is_implicit ? _t : _t_old;
  // Upwind the segment inlet temperature on the flow direction
  const auto Tin = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tfdown[i] +
                   1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tfup[i];

  // Convective heat transfer with the fluid
  return wallHeatTransferCoefficient(t_props) * _wetted_perimeter.value(_t, Point()) / 2.0 *
         (_Tf[i] + Tin - 2 * Base::_u[i]);
}

Real
ADPipeInnerWallTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
