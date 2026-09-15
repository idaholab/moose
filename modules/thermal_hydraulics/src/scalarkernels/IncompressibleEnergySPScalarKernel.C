//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleEnergySPScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", IncompressibleEnergySPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADIncompressibleEnergySPScalarKernel);

template <bool is_ad>
InputParameters
IncompressibleEnergySPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params =
      is_ad ? ADScalarTimeDerivative::validParams() : ODETimeDerivative::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic energy solve over a 1D flow path segment."
      "Assumes incompressibility locally."
      "Neglects axial conduction in fluid."
      "This version utilizes the Single Component, Single Phase fluid properties base class.");
  // Lots of inputs so we need to be clear what is what
  // This block defines coupled state variables the kernel relies on
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate in component. Takes a "
                       "scalar variable name");
  params.addCoupledVar("inlet_temperature",
                       {},
                       "Fluid temperature of nominal inlet segment/component (N-1). Takes a "
                       "scalar variable name");
  params.addCoupledVar("outlet_temperature",
                       {},
                       "Fluid temperature of nominal outlet segment/component (N+1). Takes a "
                       "scalar variable name");
  params.addCoupledVar("wall_temperature",
                       {},
                       "Wall temperature adjacent to fluid. Takes a "
                       "scalar variable name");

  // This block grabs boolean parameters that control the solve type
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  // This block characterizes the geometry and fluid type
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addRequiredParam<MooseFunctorName>("area", "Segment/Component flow area [m^2]");
  params.addRequiredParam<MooseFunctorName>("perimeter", "Segment/Component wetted perimeter [m]");
  params.addRequiredParam<MooseFunctorName>("length", "Segment/Component length [m]");

  return params;
}

template <bool is_ad>
IncompressibleEnergySPScalarKernelTempl<is_ad>::IncompressibleEnergySPScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    // Lots of inputs so we need to be clear what is what
    // This block defines coupled state variables the kernel relies on
    _m(ScalarCoupleable::coupledScalarValue("mass_flow_rate")),
    _Tup(ScalarCoupleable::coupledScalarValue("inlet_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("outlet_temperature")),
    _Tw(ScalarCoupleable::coupledScalarValue("wall_temperature")),
    // This block grabs boolean parameters that control the solve type
    _is_implicit(this->template getParam<bool>("is_implicit")),
    // This block characterizes the geometry and fluid type
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    _area(this->template getFunctor<GenericReal<is_ad>>("area")),
    _perimeter(this->template getFunctor<GenericReal<is_ad>>("perimeter")),
    _length(this->template getFunctor<GenericReal<is_ad>>("length"))
{
}

template <bool is_ad>
GenericReal<is_ad>
IncompressibleEnergySPScalarKernelTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> energy_residual = 0;
  const Moose::ElemArg _qp = Moose::ElemArg();
  const int _i = 0;
  const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting fluid properties
  auto _in = 1.0 / 2.0 * (1 - abs(_m[_i]) / _m[_i]) * _Tdown[_i] +
             1.0 / 2.0 * (1 + abs(_m[_i]) / _m[_i]) * _Tup[_i];
  auto _mu = _fp.mu_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
  auto _rho = _fp.rho_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
  auto _cp = _fp.cp_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
  auto _k = _fp.k_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);

  // Decide flow regime for HTC
  auto _Dh = 4.0 * _area(_qp, _state) / _perimeter(_qp, _state);
  auto _G = abs(_m[_i]) / _area(_qp, _state);
  auto _Re = _G * _Dh / _mu;
  auto _Pr = _mu * _cp / _k;
  // Heat transfer to fluid (Dittus-Boelter)
  auto _h = 0.023 * pow(_Re, 0.8) * pow(_Pr, 0.4) * _k / _Dh;
  auto _q = _h * _perimeter(_qp, _state) / 2.0 * (2.0 * _Tw[_i] - Base::_u[_i] - _in);
  // Advection component
  energy_residual += (_m[_i] / 2.0 * (1 - abs(_m[_i]) / _m[_i]) * _cp * _Tdown[_i] -
                      _m[_i] / 2.0 * (1 + abs(_m[_i]) / _m[_i]) * _cp * _Tup[_i] +
                      abs(_m[_i]) * _cp * Base::_u[_i]) /
                     _length(_qp, _state);
  // Wall heat transfer
  energy_residual -= _q;
  // Transient term
  energy_residual += _area(_qp, _state) * _rho * _cp * Base::_u_dot[_i];

  return energy_residual;
}

template <bool is_ad>
Real
IncompressibleEnergySPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real energy_residual = 0;
    const Moose::ElemArg _qp = Moose::ElemArg();
    const int _i = 0;
    const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting fluid properties
    auto _in = 1.0 / 2.0 * (1 - abs(_m[_i]) / _m[_i]) * _Tdown[_i] +
               1.0 / 2.0 * (1 + abs(_m[_i]) / _m[_i]) * _Tup[_i];
    auto _mu = _fp.mu_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
    auto _rho = _fp.rho_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
    auto _cp = _fp.cp_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
    auto _k = _fp.k_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);

    // Decide flow regime for HTC
    auto _Dh = 4.0 * _area(_qp, _state) / _perimeter(_qp, _state);
    auto _G = abs(_m[_i]) / _area(_qp, _state);
    auto _Re = _G * _Dh / _mu;
    auto _Pr = _mu * _cp / _k;
    // Heat transfer to fluid (Dittus-Boelter)
    auto _h = 0.023 * pow(_Re, 0.8) * pow(_Pr, 0.4) * _k / _Dh;
    auto _q = -_h * _perimeter(_qp, _state) / 2.0;
    // Advection component
    energy_residual += abs(_m[_i]) * _cp / _length(_qp, _state);
    // Wall heat transfer
    energy_residual += -_q;
    // Transient term
    energy_residual += _area(_qp, _state) * _rho * _cp * Base::_du_dot_du[_i];

    return energy_residual;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
IncompressibleEnergySPScalarKernelTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class IncompressibleEnergySPScalarKernelTempl<false>;
template class IncompressibleEnergySPScalarKernelTempl<true>;
