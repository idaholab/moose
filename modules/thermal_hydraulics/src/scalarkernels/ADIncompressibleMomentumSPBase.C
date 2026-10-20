//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIncompressibleMomentumSPBase.h"

#include "ADFunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"
#include "PhysicalConstants.h"

InputParameters
ADIncompressibleMomentumSPBase::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params += ADFunctorInterface::validParams();
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
      "height_changes",
      std::vector<MooseFunctorName>({}),
      "Height change from inlet to outlet of each segment. Takes a vector of functors.");
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
  params.addParam<Point>("functor_evaluation_point",
                         Point(),
                         "Point at which spatially varying functors (functions, field variables, "
                         "functor material properties) are evaluated");

  return params;
}

ADIncompressibleMomentumSPBase::ADIncompressibleMomentumSPBase(const InputParameters & parameters)
  : Base(parameters),
    ADFunctorInterface(this),
    _n_temps(ScalarCoupleable::coupledScalarComponents("temperatures")),
    _T(_n_temps),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _Pref(this->template getFunctor<ADReal>("reference_pressure")),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _n_segments(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _areas(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _perimeters(this->template getParam<std::vector<MooseFunctorName>>("perimeters").size()),
    _lengths(this->template getParam<std::vector<MooseFunctorName>>("lengths").size()),
    _dHs(this->template getParam<std::vector<MooseFunctorName>>("height_changes").size()),
    _forms_losses(this->template getParam<std::vector<MooseFunctorName>>("forms_losses").size()),
    _dPps(this->template getParam<std::vector<MooseFunctorName>>("pump_pressures").size()),
    _roughnesses(this->template getParam<std::vector<MooseFunctorName>>("roughnesses").size()),
    _gravity(this->template getFunctor<ADReal>("g")),
    _functor_point(getParam<Point>("functor_evaluation_point"))
{
  const auto & area_names = MooseBase::getParam<std::vector<MooseFunctorName>>("areas");
  const auto & perimeter_names = MooseBase::getParam<std::vector<MooseFunctorName>>("perimeters");
  const auto & length_names = MooseBase::getParam<std::vector<MooseFunctorName>>("lengths");
  const auto & dH_names = MooseBase::getParam<std::vector<MooseFunctorName>>("height_changes");
  const auto & forms_loss_names =
      MooseBase::getParam<std::vector<MooseFunctorName>>("forms_losses");
  const auto & dPp_names = MooseBase::getParam<std::vector<MooseFunctorName>>("pump_pressures");
  const auto & roughness_names = MooseBase::getParam<std::vector<MooseFunctorName>>("roughnesses");
  if (_n_segments != area_names.size() || _n_segments != perimeter_names.size() ||
      _n_segments != length_names.size() || _n_segments != dH_names.size() ||
      _n_segments != forms_loss_names.size() || _n_segments != dPp_names.size() ||
      _n_segments != roughness_names.size() || _n_segments != _n_temps)
  {
    mooseError(
        "Must provide consistent number of segments for each parameter! Including temperatures!");
  }
  for (const auto j : make_range(_n_segments))
  {
    _T[j] = &(ScalarCoupleable::adCoupledScalarValue("temperatures", j));
    _areas[j] = &(this->template getFunctor<ADReal>(area_names[j]));
    _perimeters[j] = &(this->template getFunctor<ADReal>(perimeter_names[j]));
    _lengths[j] = &(this->template getFunctor<ADReal>(length_names[j]));
    _dHs[j] = &(this->template getFunctor<ADReal>(dH_names[j]));
    _forms_losses[j] = &(this->template getFunctor<ADReal>(forms_loss_names[j]));
    _dPps[j] = &(this->template getFunctor<ADReal>(dPp_names[j]));
    _roughnesses[j] = &(this->template getFunctor<ADReal>(roughness_names[j]));
  }
}

void
ADIncompressibleMomentumSPBase::initialSetup()
{
  ADScalarKernel::initialSetup();

  // Locate the element that spatially varying functors will be evaluated on
  auto pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();
  _functor_elem = (*pl)(_functor_point);
}

ADReal
ADIncompressibleMomentumSPBase::computeQpResidual()
{
  ADReal momentum_residual = 0;
  const auto qp = functorArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting global fluid properties
  const auto Tave = ((*(_T[0]))[i] + (*(_T[_n_temps - 1]))[i]) / 2;
  const auto mu = _fp.mu_from_p_T(_Pref(qp, state), Tave);
  const auto rhog = _fp.rho_from_p_T(_Pref(qp, state), Tave);
  // Global rescale factor so the mass flow rate's transient term has a unit coefficient,
  // matching the coefficient of ODETimeDerivative kernel: the path's lumped L_over_A_sum is
  // Sum_j(length_j / area_j), so the whole equation is divided through by that single sum.
  ADReal L_over_A_sum = 0;
  for (const auto j : make_range(_n_segments))
    L_over_A_sum += (*(_lengths[j]))(qp, state) / (*(_areas[j]))(qp, state);
  const auto inv_L_over_A_sum = 1.0 / L_over_A_sum;
  // loop over segments
  for (const auto j : make_range(_n_segments))
  {
    // Decide flow regime for friction factor
    const auto Dh = 4.0 * (*(_areas[j]))(qp, state) / (*(_perimeters[j]))(qp, state);
    const auto G = massFlowRate() / (*(_areas[j]))(qp, state);
    const auto fd = computeFrictionFactor(mu, G, Dh, j);
    // Friction
    momentum_residual += fd * (*(_lengths[j]))(qp, state) / Dh * G * abs(G) / 2.0 / rhog;
    // Form losses
    momentum_residual += (*(_forms_losses[j]))(qp, state) * G * abs(G) / 2.0 / rhog;
    // Gravity
    // get local density for natural circulation aspect
    auto rhol = _fp.rho_from_p_T(_Pref(qp, state), (*(_T[j]))[i]);
    momentum_residual += rhol * _gravity(qp, state) * (*(_dHs[j]))(qp, state);
    // Pump pressure
    momentum_residual -= (*(_dPps[j]))(qp, state);
  }
  // Pressure drop (single path-wide unknown, applied once)
  momentum_residual += pressureDrop();

  return momentum_residual * inv_L_over_A_sum;
}

Real
ADIncompressibleMomentumSPBase::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

ADReal
ADIncompressibleMomentumSPBase::computeFrictionFactor(const ADReal & mu,
                                                      const ADReal & G,
                                                      const ADReal & Dh,
                                                      const unsigned int j)
{
  const auto qp = functorArg();
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  const auto Re = abs(G) * Dh / mu;
  const auto f_lam = 64.0 / Re;
  const auto f_turb =
      0.25 / pow((log10((*(_roughnesses[j]))(qp, state) / (Dh * 3.7) + 5.74 / pow(Re, 0.9))), 2);
  if (Re < 2300.0) // laminar
    return f_lam;
  else if (Re > 4000.0) // turbulent using Swamee-Jain approx. of Colebrook-White eq.
    return f_turb;
  else // transition, conservative interpolation between the two
    return std::max((f_turb - f_lam) / 1700 * Re + f_lam, std::max(f_lam, f_turb));
}
