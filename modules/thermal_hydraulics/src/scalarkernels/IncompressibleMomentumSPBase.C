//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleMomentumSPBase.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"
#include "PhysicalConstants.h"

template <bool is_ad>
InputParameters
IncompressibleMomentumSPBaseTempl<is_ad>::validParams()
{
  InputParameters params = is_ad ? ADScalarKernel::validParams() : ScalarKernel::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription("Base class for path-integrated incompressible momentum kernels.");
  params.addCoupledVar("temperatures",
                       {},
                       "Fluid temperature in each segment of this component. Takes a "
                       "list of scalar variable names");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<std::vector<MooseFunctorName>>(
      "areas",
      std::vector<MooseFunctorName>({}),
      "Component flow areas per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "perimeters",
      std::vector<MooseFunctorName>({}),
      "Component flow perimeters per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "lengths",
      std::vector<MooseFunctorName>({}),
      "Component flow lengths per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "alphas",
      std::vector<MooseFunctorName>({}),
      "Component flow angles per segment with respect to horizontal (-pi/2 downward to pi/2 "
      "upward). Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "forms_losses",
      std::vector<MooseFunctorName>({}),
      "Forms loss coefficients per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "pump_pressures",
      std::vector<MooseFunctorName>({}),
      "Pump pressure gains per segment [Pa]. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "roughnesses",
      std::vector<MooseFunctorName>({}),
      "Component wall roughnesses per segment [m]. Takes a vector of functors.");
  params.addParam<MooseFunctorName>(
      "g", PhysicalConstants::acceleration_of_gravity, "Gravitational acceleration [m/s]");

  return params;
}

template <bool is_ad>
IncompressibleMomentumSPBaseTempl<is_ad>::IncompressibleMomentumSPBaseTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _n_temps(ScalarCoupleable::coupledScalarComponents("temperatures")),
    _T(_n_temps),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _n_segments(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _areas(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _perimeters(this->template getParam<std::vector<MooseFunctorName>>("perimeters").size()),
    _lengths(this->template getParam<std::vector<MooseFunctorName>>("lengths").size()),
    _alphas(this->template getParam<std::vector<MooseFunctorName>>("alphas").size()),
    _forms_losses(this->template getParam<std::vector<MooseFunctorName>>("forms_losses").size()),
    _dPps(this->template getParam<std::vector<MooseFunctorName>>("pump_pressures").size()),
    _roughnesses(this->template getParam<std::vector<MooseFunctorName>>("roughnesses").size()),
    _gravity(this->template getFunctor<GenericReal<is_ad>>("g"))
{
  auto & area_names = MooseBase::getParam<std::vector<MooseFunctorName>>("areas");
  auto & perimeter_names = MooseBase::getParam<std::vector<MooseFunctorName>>("perimeters");
  auto & length_names = MooseBase::getParam<std::vector<MooseFunctorName>>("lengths");
  auto & alpha_names = MooseBase::getParam<std::vector<MooseFunctorName>>("alphas");
  auto & forms_loss_names = MooseBase::getParam<std::vector<MooseFunctorName>>("forms_losses");
  auto & dPp_names = MooseBase::getParam<std::vector<MooseFunctorName>>("pump_pressures");
  auto & roughness_names = MooseBase::getParam<std::vector<MooseFunctorName>>("roughnesses");
  if (_n_segments != area_names.size() || _n_segments != perimeter_names.size() ||
      _n_segments != length_names.size() || _n_segments != alpha_names.size() ||
      _n_segments != forms_loss_names.size() || _n_segments != dPp_names.size() ||
      _n_segments != roughness_names.size() || _n_segments != _n_temps)
  {
    mooseError(
        "Must provide consistent number of segments for each parameter! Including temperatures!");
  }
  for (const auto j : make_range(_n_segments))
  {
    _T[j] = &(ScalarCoupleable::coupledScalarValue("temperatures", j));
    _areas[j] = &(this->template getFunctor<GenericReal<is_ad>>(area_names[j]));
    _perimeters[j] = &(this->template getFunctor<GenericReal<is_ad>>(perimeter_names[j]));
    _lengths[j] = &(this->template getFunctor<GenericReal<is_ad>>(length_names[j]));
    _alphas[j] = &(this->template getFunctor<GenericReal<is_ad>>(alpha_names[j]));
    _forms_losses[j] = &(this->template getFunctor<GenericReal<is_ad>>(forms_loss_names[j]));
    _dPps[j] = &(this->template getFunctor<GenericReal<is_ad>>(dPp_names[j]));
    _roughnesses[j] = &(this->template getFunctor<GenericReal<is_ad>>(roughness_names[j]));
  }
}

template <bool is_ad>
GenericReal<is_ad>
IncompressibleMomentumSPBaseTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> momentum_residual = 0;
  const Moose::ElemArg qp = Moose::ElemArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting global fluid properties
  const auto Tave = ((*(_T[0]))[i] + (*(_T[_n_temps - 1]))[i]) / 2;
  const auto mu = _fp.mu_from_p_T(_Pref(qp, state), Tave);
  const auto rhog = _fp.rho_from_p_T(_Pref(qp, state), Tave);
  // Global rescale factor so the mass flow rate's transient term has a unit coefficient,
  // matching the coefficient of ODETimeDerivative kernel: the path's lumped L_over_A_sum is
  // Sum_j(length_j / area_j), so the whole equation is divided through by that single sum.
  GenericReal<is_ad> L_over_A_sum = 0;
  for (const auto j : make_range(_n_segments))
    L_over_A_sum += (*(_lengths[j]))(qp, state) / (*(_areas[j]))(qp, state);
  const auto inv_L_over_A_sum = 1.0 / L_over_A_sum;
  // loop over segments
  for (const auto j : make_range(_n_segments))
  {
    // Decide flow regime for friction factor
    auto Dh = 4.0 * (*(_areas[j]))(qp, state) / (*(_perimeters[j]))(qp, state);
    auto G = massFlowRate() / (*(_areas[j]))(qp, state);
    auto fd = computeFrictionFactor(mu, G, Dh, j);
    // Friction
    momentum_residual += fd * (*(_lengths[j]))(qp, state) / Dh * G * abs(G) / 2.0 / rhog;
    // Form losses
    momentum_residual += (*(_forms_losses[j]))(qp, state) * G * abs(G) / 2.0 / rhog;
    // Gravity
    // get local density for natural circulation aspect
    auto rhol = _fp.rho_from_p_T(_Pref(qp, state), (*(_T[j]))[i]);
    momentum_residual +=
        rhol * _gravity(qp, state) * (*(_lengths[j]))(qp, state) * sin((*(_alphas[j]))(qp, state));
    // Pump pressure
    momentum_residual -= (*(_dPps[j]))(qp, state);
  }
  // Pressure drop (single path-wide unknown, applied once)
  momentum_residual += pressureDrop();

  return momentum_residual * inv_L_over_A_sum;
}

template <bool is_ad>
Real
IncompressibleMomentumSPBaseTempl<is_ad>::computeQpJacobianMDot()
{
  if constexpr (!is_ad)
  {
    Real momentum_jacob = 0;
    const Moose::ElemArg qp = Moose::ElemArg();
    const int i = 0;
    const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting global fluid properties
    const auto Tave = ((*(_T[0]))[i] + (*(_T[_n_temps - 1]))[i]) / 2;
    const auto mu = _fp.mu_from_p_T(_Pref(qp, state), Tave);
    const auto rhog = _fp.rho_from_p_T(_Pref(qp, state), Tave);
    // Global rescale factor, see computeQpResidual()
    Real L_over_A_sum = 0;
    for (const auto j : make_range(_n_segments))
      L_over_A_sum += (*(_lengths[j]))(qp, state) / (*(_areas[j]))(qp, state);
    const auto inv_L_over_A_sum = 1.0 / L_over_A_sum;
    // loop over segments
    for (const auto j : make_range(_n_segments))
    {
      // Decide flow regime for friction factor
      auto Dh = 4.0 * (*(_areas[j]))(qp, state) / (*(_perimeters[j]))(qp, state);
      auto G = massFlowRate() / (*(_areas[j]))(qp, state);
      auto fd = computeFrictionFactor(mu, G, Dh, j);
      // Friction
      momentum_jacob +=
          fd * (*(_lengths[j]))(qp, state) / Dh * G / rhog / (*(_areas[j]))(qp, state);
      // Form losses
      momentum_jacob += (*(_forms_losses[j]))(qp, state) * G / rhog / (*(_areas[j]))(qp, state);
    }

    return momentum_jacob * inv_L_over_A_sum;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <bool is_ad>
Real
IncompressibleMomentumSPBaseTempl<is_ad>::computeQpJacobianDP()
{
  if constexpr (!is_ad)
  {
    const Moose::ElemArg qp = Moose::ElemArg();
    const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
    // Global rescale factor, see computeQpResidual()
    Real L_over_A_sum = 0;
    for (const auto j : make_range(_n_segments))
      L_over_A_sum += (*(_lengths[j]))(qp, state) / (*(_areas[j]))(qp, state);
    const auto inv_L_over_A_sum = 1.0 / L_over_A_sum;
    // reference pressure drop is subtracted once, scaled by inv_L_over_A_sum, so its derivative is
    // -inv_L_over_A_sum
    return -inv_L_over_A_sum;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <bool is_ad>
Real
IncompressibleMomentumSPBaseTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    return 0;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
IncompressibleMomentumSPBaseTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
IncompressibleMomentumSPBaseTempl<is_ad>::computeFrictionFactor(const GenericReal<is_ad> & mu,
                                                                const GenericReal<is_ad> & G,
                                                                const GenericReal<is_ad> & Dh,
                                                                const unsigned int j)
{
  const Moose::ElemArg qp = Moose::ElemArg();
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  const auto Re = abs(G) * Dh / mu;
  const auto lam = 64.0 / Re;
  const auto turb =
      0.25 / pow((log10((*(_roughnesses[j]))(qp, state) / (Dh * 3.7) + 5.74 / pow(Re, 0.9))), 2);
  auto fd = 64.0 / Re;
  auto pfd = &fd;
  if (Re < 2300.0) // laminar
  {
    *pfd = lam;
  }
  else if (Re > 4000.0) // turbulent using Swamee-Jain approx. of Colebrook-White eq.
  {
    *pfd = turb;
  }
  else // transition, conservative interpolation between the two
  {
    *pfd = (turb - lam) / 1700 * Re + lam;
    *pfd = std::max(*pfd, std::max(lam, turb));
  }

  return fd;
}

template class IncompressibleMomentumSPBaseTempl<false>;
template class IncompressibleMomentumSPBaseTempl<true>;
