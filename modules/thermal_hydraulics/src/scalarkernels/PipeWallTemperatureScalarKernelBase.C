//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeWallTemperatureScalarKernelBase.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "ThermalSolidProperties.h"

template <bool is_ad>
InputParameters
PipeWallTemperatureScalarKernelBaseTempl<is_ad>::validParams()
{
  InputParameters params = is_ad ? ADScalarKernel::validParams() : ScalarKernel::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Base class for solving the temperature of a radial node of a pipe wall, with axial "
      "conduction to the upstream/downstream nodes of the same radial layer and radial "
      "conduction to the node on the other side of the wall.");
  params.addCoupledVar("upstream_wall_temperature",
                       {},
                       "Wall temperature of the upstream node (N-1), same radial layer. Takes a "
                       "scalar variable name");
  params.addCoupledVar("downstream_wall_temperature",
                       {},
                       "Wall temperature of the downstream node (N+1), same radial layer. Takes "
                       "a scalar variable name");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addRequiredParam<MooseFunctorName>("area",
                                            "Cross-sectional area of this wall layer [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "interface_perimeter",
      "Perimeter of the interface between the inner and outer wall nodes [m]");
  params.addRequiredParam<MooseFunctorName>(
      "interface_thickness",
      "Radial conduction-path distance between the inner and outer wall nodes [m]");
  params.addRequiredParam<MooseFunctorName>("length",
                                            "Axial length of this node's control volume [m]");
  params.addRequiredParam<MooseFunctorName>(
      "upstream_spacing", "Axial distance from this node to the upstream node [m]");
  params.addRequiredParam<MooseFunctorName>(
      "downstream_spacing", "Axial distance from this node to the downstream node [m]");
  params.addRequiredParam<MooseFunctorName>(
      "upstream_area", "Cross-sectional area of the upstream node's wall layer [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "downstream_area", "Cross-sectional area of the downstream node's wall layer [m^2]");

  return params;
}

template <bool is_ad>
PipeWallTemperatureScalarKernelBaseTempl<is_ad>::PipeWallTemperatureScalarKernelBaseTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _sp(this->template getUserObject<ThermalSolidProperties>("sp")),
    _Tup(ScalarCoupleable::coupledScalarValue("upstream_wall_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("downstream_wall_temperature")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _area(this->template getFunctor<GenericReal<is_ad>>("area")),
    _interface_perimeter(this->template getFunctor<GenericReal<is_ad>>("interface_perimeter")),
    _interface_thickness(this->template getFunctor<GenericReal<is_ad>>("interface_thickness")),
    _length(this->template getFunctor<GenericReal<is_ad>>("length")),
    _upstream_spacing(this->template getFunctor<GenericReal<is_ad>>("upstream_spacing")),
    _downstream_spacing(this->template getFunctor<GenericReal<is_ad>>("downstream_spacing")),
    _upstream_area(this->template getFunctor<GenericReal<is_ad>>("upstream_area")),
    _downstream_area(this->template getFunctor<GenericReal<is_ad>>("downstream_area"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeWallTemperatureScalarKernelBaseTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> wall_residual = 0;
  const Moose::ElemArg qp = Moose::ElemArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();

  const auto A = _area(qp, state);
  const auto k = _sp.k_from_T(Base::_u[i]);
  const auto cp = _sp.cp_from_T(Base::_u[i]);
  const auto rho = _sp.rho_from_T(Base::_u[i]);

  // Neighbor wall-layer cross-sectional areas and conductivities, used below to build a
  // harmonic-mean face conductance for axial conduction between control volumes whose
  // cross-section may differ (two half-length resistances in series)
  const auto A_up = _upstream_area(qp, state);
  const auto k_up = _sp.k_from_T(_Tup[i]);
  const auto A_down = _downstream_area(qp, state);
  const auto k_down = _sp.k_from_T(_Tdown[i]);

  const auto G_up = 1.0 / (_upstream_spacing(qp, state) / (2.0 * k * A) +
                           _upstream_spacing(qp, state) / (2.0 * k_up * A_up));
  const auto G_down = 1.0 / (_downstream_spacing(qp, state) / (2.0 * k * A) +
                             _downstream_spacing(qp, state) / (2.0 * k_down * A_down));

  // Radial conduction from the node on the other side of this layer
  const auto q_radial = k * _interface_perimeter(qp, state) *
                        (otherNodeTemperature() - Base::_u[i]) / _interface_thickness(qp, state);
  // Axial conduction from the upstream/downstream nodes of this layer
  const auto q_axial =
      (G_up * (_Tup[i] - Base::_u[i]) + G_down * (_Tdown[i] - Base::_u[i])) / _length(qp, state);
  // Heat exchanged with the fluid or ambient environment on this node's non-radial side
  const auto q_convective = convectiveResidual();

  // Radial conduction from the node on the other side of this layer
  wall_residual -= q_radial / A / rho / cp;
  // Axial conduction from the upstream/downstream nodes of this layer
  wall_residual -= q_axial / A / rho / cp;
  // Heat exchanged with the fluid or ambient environment
  wall_residual -= q_convective / A / rho / cp;

  return wall_residual;
}

template <bool is_ad>
Real
PipeWallTemperatureScalarKernelBaseTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real wall_jacob = 0;
    const Moose::ElemArg qp = Moose::ElemArg();
    const int i = 0;
    const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();

    const auto A = _area(qp, state);
    const auto k = _sp.k_from_T(Base::_u[i]);
    const auto cp = _sp.cp_from_T(Base::_u[i]);
    const auto rho = _sp.rho_from_T(Base::_u[i]);

    const auto A_up = _upstream_area(qp, state);
    const auto k_up = _sp.k_from_T(_Tup[i]);
    const auto A_down = _downstream_area(qp, state);
    const auto k_down = _sp.k_from_T(_Tdown[i]);

    const auto G_up = 1.0 / (_upstream_spacing(qp, state) / (2.0 * k * A) +
                             _upstream_spacing(qp, state) / (2.0 * k_up * A_up));
    const auto G_down = 1.0 / (_downstream_spacing(qp, state) / (2.0 * k * A) +
                               _downstream_spacing(qp, state) / (2.0 * k_down * A_down));

    const auto q_radial = -k * _interface_perimeter(qp, state) / _interface_thickness(qp, state);
    const auto q_axial = (-G_up - G_down) / _length(qp, state);
    const auto q_convective = convectiveJacobian();

    wall_jacob -= q_radial / A / rho / cp;
    wall_jacob -= q_axial / A / rho / cp;
    wall_jacob -= q_convective / A / rho / cp;

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
PipeWallTemperatureScalarKernelBaseTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class PipeWallTemperatureScalarKernelBaseTempl<false>;
template class PipeWallTemperatureScalarKernelBaseTempl<true>;
