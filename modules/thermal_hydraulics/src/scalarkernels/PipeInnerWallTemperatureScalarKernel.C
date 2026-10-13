//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeInnerWallTemperatureScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeInnerWallTemperatureScalarKernel);

InputParameters
ADPipeInnerWallTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
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
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  params.addRequiredParam<MooseFunctorName>(
      "flow_area", "Cross-sectional area of the fluid flow channel adjacent to this node [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "wetted_perimeter", "Wetted perimeter of the fluid flow channel adjacent to this node [m]");

  return params;
}

ADPipeInnerWallTemperatureScalarKernel::ADPipeInnerWallTemperatureScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tout(ScalarCoupleable::adCoupledScalarValue("outer_wall_temperature")),
    _Tf(ScalarCoupleable::adCoupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::adCoupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::adCoupledScalarValue("downstream_fluid_temperature")),
    _Pref(this->template getFunctor<ADReal>("reference_pressure")),
    _flow_area(this->template getFunctor<ADReal>("flow_area")),
    _wetted_perimeter(this->template getFunctor<ADReal>("wetted_perimeter"))
{
}

ADReal
ADPipeInnerWallTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  const Moose::ElemArg elem_arg = Moose::ElemArg();
  const int i = 0;
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting fluid properties
  const auto Tin = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tfdown[i] +
                   1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tfup[i];
  const auto muf = _fp.mu_from_p_T(_Pref(elem_arg, state), (Base::_u[i] + Tin) / 2);
  const auto cpf = _fp.cp_from_p_T(_Pref(elem_arg, state), (Base::_u[i] + Tin) / 2);
  const auto kf = _fp.k_from_p_T(_Pref(elem_arg, state), (Base::_u[i] + Tin) / 2);

  // Decide flow regime for HTC
  const auto Dh = 4.0 * _flow_area(elem_arg, state) / _wetted_perimeter(elem_arg, state);
  const auto G = abs(_m[i]) / _flow_area(elem_arg, state);
  const auto Re = G * Dh / muf;
  const auto Pr = muf * cpf / kf;
  // Heat transfer to fluid (Dittus-Boelter)
  const auto h = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4) * kf / Dh;

  // Convective heat transfer with the fluid
  return h * _wetted_perimeter(elem_arg, state) / 2.0 * (_Tf[i] + Tin - 2 * Base::_u[i]);
}

Real
ADPipeInnerWallTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
