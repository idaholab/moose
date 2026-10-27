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
      true,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp",
      "Fluid properties user object(s). Supply either one (applied to all segments) "
      "or one per segment.");
  params.addParam<UserObjectName>("reference_fp",
                                  "Fluid properties of the liquid phase for draining problems.");
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
    _n_fp(this->template getParam<std::vector<UserObjectName>>("fp").size()),
    _n_segments(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _fp(_n_segments), // size it here, or after _n_segments if declared later in the header
    _fp_ref(isParamValid("reference_fp") ? &getUserObjectByName<SinglePhaseFluidProperties>(
                                               getParam<UserObjectName>("reference_fp"))
                                         : nullptr),
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
  const auto & fp_names = MooseBase::getParam<std::vector<UserObjectName>>("fp");
  if (_n_fp != 1 && _n_fp != _n_segments)
    paramError(
        "fp", "Provide either one fluid properties object or one per segment (", _n_segments, ").");

  for (const auto j : make_range(_n_segments))
  {
    const auto & name = (_n_fp == 1) ? fp_names[0] : fp_names[j];
    _fp[j] = &getUserObjectByName<SinglePhaseFluidProperties>(name);
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
  const auto P = _Pref(qp, state);

  // Inertia scale: Sum_j (rho_j / rho_ref,j) L_j / A_j. Reduces to Sum_j L_j / A_j when
  // every segment holds the reference liquid. Gas-filled segments carry ~no inertia.
  ADReal L_over_A_sum = 0;
  for (const auto j : make_range(_n_segments))
  {
    const auto Tj = (*(_T[j]))[i];
    const auto rho_j = _fp[j]->rho_from_p_T(P, Tj);
    const auto rho_ref = _fp_ref ? _fp_ref->rho_from_p_T(P, Tj) : rho_j;
    L_over_A_sum +=
        max(rho_j / rho_ref, 1e-6) * (*(_lengths[j]))(qp, state) / (*(_areas[j]))(qp, state);
  }
  const auto inv_L_over_A_sum = 1.0 / L_over_A_sum;

  for (const auto j : make_range(_n_segments))
  {
    const auto Tj = (*(_T[j]))[i];
    const auto A = (*(_areas[j]))(qp, state);
    const auto Dh = 4.0 * A / (*(_perimeters[j]))(qp, state);
    // Segment properties (blended liquid/gas) and liquid reference density
    const auto mu = _fp[j]->mu_from_p_T(P, Tj);
    const auto rho_j = _fp[j]->rho_from_p_T(P, Tj);
    const auto rho_ref = _fp_ref ? _fp_ref->rho_from_p_T(P, Tj) : rho_j;
    // Velocity is continuous along the path (volumetric continuity); the mass flux
    // actually present in this segment is rho_j * v.
    const auto v = massFlowRate() / (rho_ref * A);
    const auto Gj = rho_j * v;
    const auto fd = computeFrictionFactor(mu, Gj, Dh, j);
    // Friction and form losses: f L/Dh * rho_j v|v| / 2  (-> 0 as rho_j -> 0)
    momentum_residual += fd * (*(_lengths[j]))(qp, state) / Dh * Gj * abs(Gj) / 2.0 / rho_j;
    momentum_residual += (*(_forms_losses[j]))(qp, state) * Gj * abs(Gj) / 2.0 / rho_j;
    // Gravity: hydrostatic head of whatever is in the segment
    momentum_residual += rho_j * _gravity(qp, state) * (*(_dHs[j]))(qp, state);
    // Pump
    momentum_residual -= (*(_dPps[j]))(qp, state);
  }
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
  const auto Re = max(abs(G) * Dh / mu, 1.0e-03);
  if (Re < 2300.0) // laminar
    return 64.0 / Re;
  else if (Re > 4000.0) // turbulent using Swamee-Jain approx. of Colebrook-White eq.
    return 0.25 /
           pow((log10((*(_roughnesses[j]))(qp, state) / (Dh * 3.7) + 5.74 / pow(Re, 0.9))), 2);
  else // transition, conservative interpolation between the two
  {
    const auto f_lam_2300 = 64.0 / 2300.0;
    const auto f_turb_4000 =
        0.25 /
        pow(log10((*(_roughnesses[j]))(qp, state) / (Dh * 3.7) + 5.74 / pow(4000.0, 0.9)), 2);
    return f_lam_2300 + (f_turb_4000 - f_lam_2300) * (Re - 2300.0) / 1700.0;
  }
}
