//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeOuterWallCoupledConvectiveTemperatureScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", PipeOuterWallCoupledConvectiveTemperatureScalarKernel);
registerMooseObject("ThermalHydraulicsApp",
                    ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel);

template <bool is_ad>
InputParameters
PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall, "
      "convectively coupled to a secondary-side flowing fluid, such as the shell side of a "
      "heat exchanger.");
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate of the secondary-side fluid. Takes a "
                       "scalar variable name");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addCoupledVar("fluid_temperature",
                       {},
                       "Secondary-side fluid temperature adjacent to this wall node. Takes a "
                       "scalar variable name");
  params.addCoupledVar(
      "upstream_fluid_temperature",
      {},
      "Secondary-side fluid temperature adjacent to the upstream wall node. Takes a scalar "
      "variable name");
  params.addCoupledVar(
      "downstream_fluid_temperature",
      {},
      "Secondary-side fluid temperature adjacent to the downstream wall node. Takes a scalar "
      "variable name");
  params.addRequiredParam<MooseFunctorName>("reference_pressure",
                                            "Secondary-side system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>(
      "fp", "The name of the user object for secondary-side fluid properties");
  params.addRequiredParam<MooseFunctorName>(
      "flow_area",
      "Cross-sectional area of the secondary-side flow channel adjacent to this node [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "wetted_perimeter",
      "Wetted perimeter of the secondary-side flow channel adjacent to this node [m]");

  return params;
}

template <bool is_ad>
PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<is_ad>::
    PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl(const InputParameters & parameters)
  : Base(parameters),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _m(ScalarCoupleable::coupledScalarValue("mass_flow_rate")),
    _Tin(ScalarCoupleable::coupledScalarValue("inner_wall_temperature")),
    _Tf(ScalarCoupleable::coupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::coupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::coupledScalarValue("downstream_fluid_temperature")),
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    _flow_area(this->template getFunctor<GenericReal<is_ad>>("flow_area")),
    _wetted_perimeter(this->template getFunctor<GenericReal<is_ad>>("wetted_perimeter"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<is_ad>::convectiveResidual() const
{
  const Moose::ElemArg qp = Moose::ElemArg();
  const int i = 0;
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting fluid properties
  const auto in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tfdown[i] +
                  1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tfup[i];
  const auto muf = _fp.mu_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
  const auto cpf = _fp.cp_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
  const auto kf = _fp.k_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);

  // Decide flow regime for HTC
  const auto Dh = 4.0 * _flow_area(qp, state) / _wetted_perimeter(qp, state);
  const auto G = abs(_m[i]) / _flow_area(qp, state);
  const auto Re = G * Dh / muf;
  const auto Pr = muf * cpf / kf;
  // Heat transfer to fluid (Dittus-Boelter)
  const auto h = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4) * kf / Dh;

  // Convective heat transfer with the secondary-side fluid
  return h * _wetted_perimeter(qp, state) / 2.0 * (_Tf[i] + in - 2 * Base::_u[i]);
}

template <bool is_ad>
Real
PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<is_ad>::convectiveJacobian() const
{
  if constexpr (!is_ad)
  {
    const Moose::ElemArg qp = Moose::ElemArg();
    const int i = 0;
    const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting fluid properties
    const auto in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tfdown[i] +
                    1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tfup[i];
    const auto muf = _fp.mu_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
    const auto cpf = _fp.cp_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
    const auto kf = _fp.k_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);

    // Decide flow regime for HTC
    const auto Dh = 4.0 * _flow_area(qp, state) / _wetted_perimeter(qp, state);
    const auto G = abs(_m[i]) / _flow_area(qp, state);
    const auto Re = G * Dh / muf;
    const auto Pr = muf * cpf / kf;
    // Heat transfer to fluid (Dittus-Boelter)
    const auto h = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4) * kf / Dh;

    // Convective heat transfer with the secondary-side fluid
    return -h * _wetted_perimeter(qp, state);
  }
  else
  {
    mooseError("convectiveJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<true>::convectiveJacobian() const
{
  mooseError("Internal error, calling convectiveJacobian in AD class.");
  return 0.0;
}

template class PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<false>;
template class PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<true>;
