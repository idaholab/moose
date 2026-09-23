//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeInnerWallTemperatureScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "ThermalSolidProperties.h"

registerMooseObject("ThermalHydraulicsApp", PipeInnerWallTemperatureScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADPipeInnerWallTemperatureScalarKernel);

template <bool is_ad>
InputParameters
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params =
      is_ad ? ADScalarTimeDerivative::validParams() : ODETimeDerivative::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Solves for the temperature of the inner-surface radial node of a pipe wall.");
  // This block defines coupled state variables the kernel relies on
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate in component. Takes a "
                       "scalar variable name");
  params.addCoupledVar("outer_wall_temperature",
                       {},
                       "Temperature of the outer-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addCoupledVar("upstream_wall_temperature",
                       {},
                       "Inner-surface wall temperature of the upstream node (N-1). Takes a "
                       "scalar variable name");
  params.addCoupledVar("downstream_wall_temperature",
                       {},
                       "Inner-surface wall temperature of the downstream node (N+1). Takes a "
                       "scalar variable name");
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
  // This block grabs boolean parameters that control the solve type
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  // This block characterizes the solid properties
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  // This block characterizes the fluid properties
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  // This block characterizes the geometry, which may vary axially
  params.addRequiredParam<MooseFunctorName>("inner_diameter", "Pipe inner diameter [m]");
  params.addRequiredParam<MooseFunctorName>("outer_diameter", "Pipe outer diameter [m]");
  params.addRequiredParam<MooseFunctorName>("length",
                                            "Axial length of this node's control volume [m]");
  params.addRequiredParam<MooseFunctorName>(
      "upstream_spacing", "Axial distance from this node to the upstream node [m]");
  params.addRequiredParam<MooseFunctorName>(
      "downstream_spacing", "Axial distance from this node to the downstream node [m]");
  params.addRequiredParam<MooseFunctorName>("upstream_inner_diameter",
                                            "Pipe inner diameter at the upstream node [m]");
  params.addRequiredParam<MooseFunctorName>("upstream_outer_diameter",
                                            "Pipe outer diameter at the upstream node [m]");
  params.addRequiredParam<MooseFunctorName>("downstream_inner_diameter",
                                            "Pipe inner diameter at the downstream node [m]");
  params.addRequiredParam<MooseFunctorName>("downstream_outer_diameter",
                                            "Pipe outer diameter at the downstream node [m]");

  return params;
}

template <bool is_ad>
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::PipeInnerWallTemperatureScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _sp(this->template getUserObject<ThermalSolidProperties>("sp")),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    // This block defines coupled state variables the kernel relies on
    _m(ScalarCoupleable::coupledScalarValue("mass_flow_rate")),
    _Tout(ScalarCoupleable::coupledScalarValue("outer_wall_temperature")),
    _Tup(ScalarCoupleable::coupledScalarValue("upstream_wall_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("downstream_wall_temperature")),
    _Tf(ScalarCoupleable::coupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::coupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::coupledScalarValue("downstream_fluid_temperature")),
    // This block grabs boolean parameters that control the solve type
    _is_implicit(this->template getParam<bool>("is_implicit")),
    // Fluid reference pressure
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    // This block characterizes the geometry, which may vary axially
    _inner_diameter(this->template getFunctor<GenericReal<is_ad>>("inner_diameter")),
    _outer_diameter(this->template getFunctor<GenericReal<is_ad>>("outer_diameter")),
    _length(this->template getFunctor<GenericReal<is_ad>>("length")),
    _upstream_spacing(this->template getFunctor<GenericReal<is_ad>>("upstream_spacing")),
    _downstream_spacing(this->template getFunctor<GenericReal<is_ad>>("downstream_spacing")),
    _upstream_inner_diameter(
        this->template getFunctor<GenericReal<is_ad>>("upstream_inner_diameter")),
    _upstream_outer_diameter(
        this->template getFunctor<GenericReal<is_ad>>("upstream_outer_diameter")),
    _downstream_inner_diameter(
        this->template getFunctor<GenericReal<is_ad>>("downstream_inner_diameter")),
    _downstream_outer_diameter(
        this->template getFunctor<GenericReal<is_ad>>("downstream_outer_diameter"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::computeQpResidual()
{
  const Moose::ElemArg _qp = Moose::ElemArg();
  const int _i = 0;
  const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();

  // start by getting fluid properties
  auto _in = 1.0 / 2.0 * (1 - abs(_m[_i]) / _m[_i]) * _Tfdown[_i] +
             1.0 / 2.0 * (1 + abs(_m[_i]) / _m[_i]) * _Tfup[_i];
  auto _muf = _fp.mu_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
  auto _cpf = _fp.cp_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
  auto _kf = _fp.k_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);

  // Decide flow regime for HTC
  auto _G = 4 * abs(_m[_i]) / pow(_inner_diameter(_qp, _state), 2) / libMesh::pi;
  auto _Re = _G * _inner_diameter(_qp, _state) / _muf;
  auto _Pr = _muf * _cpf / _kf;

  // Heat transfer to fluid (Dittus-Boelter)
  auto _h = 0.023 * pow(_Re, 0.8) * pow(_Pr, 0.4) * _kf / _inner_diameter(_qp, _state);

  // Cross-sectional area of this (inner) layer of the pipe wall
  auto _mid_diameter = (_outer_diameter(_qp, _state) + _inner_diameter(_qp, _state)) / 2;
  const auto _A =
      libMesh::pi / 4.0 *
      (_mid_diameter * _mid_diameter - _inner_diameter(_qp, _state) * _inner_diameter(_qp, _state));

  const auto _k = _sp.k_from_T(Base::_u[_i]);
  const auto _cp = _sp.cp_from_T(Base::_u[_i]);
  const auto _rho = _sp.rho_from_T(Base::_u[_i]);

  // Radial conduction from the outer wall node, across the full wall thickness
  const auto _q_radial = 2.0 * libMesh::pi * _k * (_Tout[_i] - Base::_u[_i]) /
                         log(_outer_diameter(_qp, _state) / _inner_diameter(_qp, _state));

  // Neighbor wall-layer cross-sectional areas and conductivities, used below to build a
  // harmonic-mean face conductance for axial conduction between control volumes whose
  // cross-section may differ (two half-length resistances in series)
  const auto _mid_diameter_up =
      (_upstream_outer_diameter(_qp, _state) + _upstream_inner_diameter(_qp, _state)) / 2;
  const auto _A_up =
      libMesh::pi / 4.0 *
      (_mid_diameter_up * _mid_diameter_up -
       _upstream_inner_diameter(_qp, _state) * _upstream_inner_diameter(_qp, _state));
  const auto _k_up = _sp.k_from_T(_Tup[_i]);

  const auto _mid_diameter_down =
      (_downstream_outer_diameter(_qp, _state) + _downstream_inner_diameter(_qp, _state)) / 2;
  const auto _A_down =
      libMesh::pi / 4.0 *
      (_mid_diameter_down * _mid_diameter_down -
       _downstream_inner_diameter(_qp, _state) * _downstream_inner_diameter(_qp, _state));
  const auto _k_down = _sp.k_from_T(_Tdown[_i]);

  const auto _G_up = 1.0 / (_upstream_spacing(_qp, _state) / (2.0 * _k * _A) +
                            _upstream_spacing(_qp, _state) / (2.0 * _k_up * _A_up));
  const auto _G_down = 1.0 / (_downstream_spacing(_qp, _state) / (2.0 * _k * _A) +
                              _downstream_spacing(_qp, _state) / (2.0 * _k_down * _A_down));

  // Axial conduction from the upstream/downstream inner-wall nodes
  const auto _q_axial =
      (_G_up * (_Tup[_i] - Base::_u[_i]) + _G_down * (_Tdown[_i] - Base::_u[_i])) /
      _length(_qp, _state);
  // Convective heat transfer with the fluid
  const auto _q_fluid = _h * 2 * _inner_diameter(_qp, _state) * (_Tf[_i] + _in - 2 * Base::_u[_i]);

  GenericReal<is_ad> wall_residual = _A * _rho * _cp * Base::_u_dot[_i];
  wall_residual -= _q_radial;
  wall_residual -= _q_axial;
  wall_residual -= _q_fluid;

  return wall_residual;
}

template <bool is_ad>
Real
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    const Moose::ElemArg _qp = Moose::ElemArg();
    const int _i = 0;
    const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();

    // start by getting fluid properties
    auto _in = 1.0 / 2.0 * (1 - abs(_m[_i]) / _m[_i]) * _Tfdown[_i] +
               1.0 / 2.0 * (1 + abs(_m[_i]) / _m[_i]) * _Tfup[_i];
    auto _muf = _fp.mu_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
    auto _cpf = _fp.cp_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);
    auto _kf = _fp.k_from_p_T(_Pref(_qp, _state), (Base::_u[_i] + _in) / 2);

    // Decide flow regime for HTC
    auto _G = 4 * abs(_m[_i]) / pow(_inner_diameter(_qp, _state), 2) / libMesh::pi;
    auto _Re = _G * _inner_diameter(_qp, _state) / _muf;
    auto _Pr = _muf * _cpf / _kf;

    // Heat transfer to fluid (Dittus-Boelter)
    auto _h = 0.023 * pow(_Re, 0.8) * pow(_Pr, 0.4) * _kf / _inner_diameter(_qp, _state);

    // Cross-sectional area of this (inner) layer of the pipe wall
    auto _mid_diameter = (_outer_diameter(_qp, _state) + _inner_diameter(_qp, _state)) / 2;
    const auto _A = libMesh::pi / 4.0 *
                    (_mid_diameter * _mid_diameter -
                     _inner_diameter(_qp, _state) * _inner_diameter(_qp, _state));

    const auto _k = _sp.k_from_T(Base::_u[_i]);
    const auto _cp = _sp.cp_from_T(Base::_u[_i]);
    const auto _rho = _sp.rho_from_T(Base::_u[_i]);

    // Radial conduction from the outer wall node, across the full wall thickness
    const auto _q_radial =
        -2.0 * libMesh::pi * _k / log(_outer_diameter(_qp, _state) / _inner_diameter(_qp, _state));

    // Neighbor wall-layer cross-sectional areas and conductivities, used below to build a
    // harmonic-mean face conductance for axial conduction between control volumes whose
    // cross-section may differ (two half-length resistances in series)
    const auto _mid_diameter_up =
        (_upstream_outer_diameter(_qp, _state) + _upstream_inner_diameter(_qp, _state)) / 2;
    const auto _A_up =
        libMesh::pi / 4.0 *
        (_mid_diameter_up * _mid_diameter_up -
         _upstream_inner_diameter(_qp, _state) * _upstream_inner_diameter(_qp, _state));
    const auto _k_up = _sp.k_from_T(_Tup[_i]);

    const auto _mid_diameter_down =
        (_downstream_outer_diameter(_qp, _state) + _downstream_inner_diameter(_qp, _state)) / 2;
    const auto _A_down =
        libMesh::pi / 4.0 *
        (_mid_diameter_down * _mid_diameter_down -
         _downstream_inner_diameter(_qp, _state) * _downstream_inner_diameter(_qp, _state));
    const auto _k_down = _sp.k_from_T(_Tdown[_i]);

    const auto _G_up = 1.0 / (_upstream_spacing(_qp, _state) / (2.0 * _k * _A) +
                              _upstream_spacing(_qp, _state) / (2.0 * _k_up * _A_up));
    const auto _G_down = 1.0 / (_downstream_spacing(_qp, _state) / (2.0 * _k * _A) +
                                _downstream_spacing(_qp, _state) / (2.0 * _k_down * _A_down));

    // Axial conduction from the upstream/downstream inner-wall nodes
    const auto _q_axial = (-_G_up - _G_down) / _length(_qp, _state);
    // Convective heat transfer with the fluid
    const auto _q_fluid = -_h * 2 * _inner_diameter(_qp, _state);

    GenericReal<is_ad> wall_jacob = _A * _rho * _cp * Base::_du_dot_du[_i];
    wall_jacob -= _q_radial;
    wall_jacob -= _q_axial;
    wall_jacob -= _q_fluid;

    return wall_jacob;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
PipeInnerWallTemperatureScalarKernelTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class PipeInnerWallTemperatureScalarKernelTempl<false>;
template class PipeInnerWallTemperatureScalarKernelTempl<true>;
