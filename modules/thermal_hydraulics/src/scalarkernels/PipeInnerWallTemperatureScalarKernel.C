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
      "Solves for the temperature of the inner-surface radial node of a pipe wall, with "
      "radial conduction to a coupled outer-surface wall node, axial conduction to coupled "
      "upstream/downstream inner-wall nodes, and convective heat transfer with the fluid.");
  // This block defines coupled state variables the kernel relies on
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
  // This block grabs boolean parameters that control the solve type
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  // This block characterizes the solid properties
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  // This block characterizes the geometry, which may vary axially
  params.addRequiredParam<MooseFunctorName>("inner_diameter", "Pipe inner diameter [m]");
  params.addRequiredParam<MooseFunctorName>("outer_diameter", "Pipe outer diameter [m]");
  params.addRequiredParam<MooseFunctorName>(
      "mid_diameter",
      "Diameter splitting the wall thickness between the inner and outer radial nodes [m]");
  params.addRequiredParam<MooseFunctorName>("length",
                                            "Axial length of this node's control volume [m]");
  params.addRequiredParam<MooseFunctorName>(
      "upstream_spacing", "Axial distance from this node to the upstream node [m]");
  params.addRequiredParam<MooseFunctorName>(
      "downstream_spacing", "Axial distance from this node to the downstream node [m]");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Fluid-side heat transfer coefficient [W/(m^2*K)]");
  params.addRequiredParam<MooseFunctorName>("wetted_perimeter", "Fluid-wetted perimeter [m]");

  return params;
}

template <bool is_ad>
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::PipeInnerWallTemperatureScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _sp(this->template getUserObject<ThermalSolidProperties>("sp")),
    // This block defines coupled state variables the kernel relies on
    _Tout(ScalarCoupleable::coupledScalarValue("outer_wall_temperature")),
    _Tup(ScalarCoupleable::coupledScalarValue("upstream_wall_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("downstream_wall_temperature")),
    _Tf(ScalarCoupleable::coupledScalarValue("fluid_temperature")),
    // This block grabs boolean parameters that control the solve type
    _is_implicit(this->template getParam<bool>("is_implicit")),
    // This block characterizes the geometry, which may vary axially
    _inner_diameter(this->template getFunctor<GenericReal<is_ad>>("inner_diameter")),
    _outer_diameter(this->template getFunctor<GenericReal<is_ad>>("outer_diameter")),
    _mid_diameter(this->template getFunctor<GenericReal<is_ad>>("mid_diameter")),
    _length(this->template getFunctor<GenericReal<is_ad>>("length")),
    _upstream_spacing(this->template getFunctor<GenericReal<is_ad>>("upstream_spacing")),
    _downstream_spacing(this->template getFunctor<GenericReal<is_ad>>("downstream_spacing")),
    _htc(this->template getFunctor<GenericReal<is_ad>>("heat_transfer_coefficient")),
    _perimeter(this->template getFunctor<GenericReal<is_ad>>("wetted_perimeter"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeInnerWallTemperatureScalarKernelTempl<is_ad>::computeQpResidual()
{
  const Moose::ElemArg _qp = Moose::ElemArg();
  const int _i = 0;
  const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();

  const auto _Di = _inner_diameter(_qp, _state);
  const auto _Do = _outer_diameter(_qp, _state);
  const auto _Dm = _mid_diameter(_qp, _state);
  const auto _L = _length(_qp, _state);
  const auto _dx_up = _upstream_spacing(_qp, _state);
  const auto _dx_down = _downstream_spacing(_qp, _state);
  const auto _h = _htc(_qp, _state);
  const auto _Pw = _perimeter(_qp, _state);

  // Cross-sectional area of this (inner) layer of the pipe wall
  const auto _A = libMesh::pi / 4.0 * (_Dm * _Dm - _Di * _Di);

  const auto _k = _sp.k_from_T(Base::_u[_i]);
  const auto _cp = _sp.cp_from_T(Base::_u[_i]);
  const auto _rho = _sp.rho_from_T(Base::_u[_i]);

  // Radial conduction from the outer wall node, across the full wall thickness
  const auto _q_radial = 2.0 * libMesh::pi * _k * (_Tout[_i] - Base::_u[_i]) / log(_Do / _Di);
  // Axial conduction from the upstream/downstream inner-wall nodes
  const auto _q_axial = (_k * _A * (_Tup[_i] - Base::_u[_i]) / _dx_up +
                         _k * _A * (_Tdown[_i] - Base::_u[_i]) / _dx_down) /
                        _L;
  // Convective heat transfer with the fluid
  const auto _q_fluid = _h * _Pw * (_Tf[_i] - Base::_u[_i]);

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

    const auto _Di = _inner_diameter(_qp, _state);
    const auto _Do = _outer_diameter(_qp, _state);
    const auto _Dm = _mid_diameter(_qp, _state);
    const auto _L = _length(_qp, _state);
    const auto _dx_up = _upstream_spacing(_qp, _state);
    const auto _dx_down = _downstream_spacing(_qp, _state);
    const auto _h = _htc(_qp, _state);
    const auto _Pw = _perimeter(_qp, _state);

    const auto _A = libMesh::pi / 4.0 * (_Dm * _Dm - _Di * _Di);
    const auto _k = _sp.k_from_T(Base::_u[_i]);
    const auto _cp = _sp.cp_from_T(Base::_u[_i]);
    const auto _rho = _sp.rho_from_T(Base::_u[_i]);

    Real wall_jacobian = _A * _rho * _cp * Base::_du_dot_du[_i];
    wall_jacobian += 2.0 * libMesh::pi * _k / log(_Do / _Di);
    wall_jacobian += _k * _A * (1.0 / _dx_up + 1.0 / _dx_down) / _L;
    wall_jacobian += _h * _Pw;

    return wall_jacobian;
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
