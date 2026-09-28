//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleEnergySPScalarKernel.h"

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
  params.addClassDescription("Implements a generic energy solve over a 1D flow path segment.");
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
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
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
    _m(ScalarCoupleable::coupledScalarValue("mass_flow_rate")),
    _Tup(ScalarCoupleable::coupledScalarValue("inlet_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("outlet_temperature")),
    _Tw(ScalarCoupleable::coupledScalarValue("wall_temperature")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
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
  const Moose::ElemArg qp = Moose::ElemArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting fluid properties
  const auto in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tdown[i] +
                  1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tup[i];
  const auto mu = _fp.mu_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
  const auto rho = _fp.rho_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
  const auto cp = _fp.cp_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
  const auto k = _fp.k_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);

  // Compute HTC quantities
  const auto Dh = 4.0 * _area(qp, state) / _perimeter(qp, state);
  const auto G = abs(_m[i]) / _area(qp, state);
  const auto Re = G * Dh / mu;
  const auto Pr = mu * cp / k;
  // Heat transfer to fluid (Dittus-Boelter)
  const auto h = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4) * k / Dh;
  const auto q = h * _perimeter(qp, state) / 2.0 * (2.0 * _Tw[i] - Base::_u[i] - in);
  // Advection component
  energy_residual += (_m[i] / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tdown[i] -
                      _m[i] / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tup[i] + abs(_m[i]) * Base::_u[i]) /
                     _length(qp, state) / _area(qp, state) / rho;
  // Wall heat transfer
  energy_residual -= q / _area(qp, state) / rho / cp;

  return energy_residual;
}

template <bool is_ad>
void
IncompressibleEnergySPScalarKernelTempl<is_ad>::reinit()
{
  // ADScalarKernel::reinit() resets its cached-Jacobian flag; ScalarKernel has no reinit()
  // for the non-AD case, so nothing needs to happen there.
  if constexpr (is_ad)
    Base::reinit();
}

template <bool is_ad>
Real
IncompressibleEnergySPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real energy_jacob = 0;
    const Moose::ElemArg qp = Moose::ElemArg();
    const int i = 0;
    const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting fluid properties
    const auto in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tdown[i] +
                    1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tup[i];
    const auto mu = _fp.mu_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
    const auto rho = _fp.rho_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
    const auto cp = _fp.cp_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);
    const auto k = _fp.k_from_p_T(_Pref(qp, state), (Base::_u[i] + in) / 2);

    // Decide flow regime for HTC
    const auto Dh = 4.0 * _area(qp, state) / _perimeter(qp, state);
    const auto G = abs(_m[i]) / _area(qp, state);
    const auto Re = G * Dh / mu;
    const auto Pr = mu * cp / k;
    // Heat transfer to fluid (Dittus-Boelter)
    const auto h = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4) * k / Dh;
    const auto q = -h * _perimeter(qp, state) / 2.0;
    // Advection component
    energy_jacob += abs(_m[i]) / _length(qp, state) / _area(qp, state) / rho;
    // Wall heat transfer
    energy_jacob -= q / _area(qp, state) / rho / cp;

    return energy_jacob;
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
