//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PInterfaceAreaSourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", WCNSFV2PInterfaceAreaSourceSink);

InputParameters
WCNSFV2PInterfaceAreaSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Source and sink of interfacial area for two-phase flow mixture model.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");

  params.addParam<MooseFunctorName>("L", 1.0, "The characteristic dissipation length.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>(NS::density + std::string("_d"),
                                            "Dispersed phase density.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "Continuous phase density.");
  params.addParam<MooseFunctorName>(
      "k_c", 0.0, "Mass exchange coefficients from continous to dispersed phases.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Fraction dispersed phase.");
  params.addParam<Real>("fd_max", 1.0, "Maximum dispersed phase fraction.");

  params.addParam<MooseFunctorName>("sigma", 1.0, "Surface tension between phases.");
  params.addParam<MooseFunctorName>("particle_diameter", 1.0, "Maximum particle diameter.");

  params.addParam<Real>("cutoff_fraction",
                        0.1,
                        "Void fraction at which the interface area density mass transfer model is "
                        "activated. Below this fraction, spherical bubbles are assumed.");

  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

WCNSFV2PInterfaceAreaSourceSink::WCNSFV2PInterfaceAreaSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _characheristic_length(getFunctor<ADReal>("L")),
    _rho_mixture(getFunctor<ADReal>(NS::density)),
    _rho_d(getFunctor<ADReal>(NS::density + std::string("_d"))),
    _pressure(getFunctor<ADReal>(NS::pressure)),
    _mass_exchange_coefficient(getFunctor<ADReal>("k_c")),
    _f_d(getFunctor<ADReal>("fd")),
    _f_d_max(getParam<Real>("fd_max")),
    _sigma(getFunctor<ADReal>("sigma")),
    _particle_diameter(getFunctor<ADReal>("particle_diameter")),
    _cutoff_fraction(getParam<Real>("cutoff_fraction"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");
}

ADReal
WCNSFV2PInterfaceAreaSourceSink::computeQpResidual()
{

  // Useful Arguments
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem);
  const bool is_transient = _subproblem.isTransient();

  // Current Variables
  const auto u = _u_var(elem_arg, state);
  const auto rho_d = _rho_d(elem_arg, state);
  const auto rho_d_grad = _rho_d.gradient(elem_arg, state);
  const auto xi = _var(elem_arg, state);
  const auto rho_m = _rho_mixture(elem_arg, state);
  const auto f_d = _f_d(elem_arg, state);
  const auto sigma = _sigma(elem_arg, state);
  const auto rho_l = (rho_m - f_d * rho_d) / (1.0 - f_d + libMesh::TOLERANCE);
  const auto complement_fd = std::max(_f_d_max - f_d, libMesh::TOLERANCE);
  const auto f_d_o_xi = f_d / (_var(elem_arg, state) + libMesh::TOLERANCE) + libMesh::TOLERANCE;
  const auto f_d_o_xi_old =
      f_d / (raw_value(_var(elem_arg, state)) + libMesh::TOLERANCE) + libMesh::TOLERANCE;

  // Adding bubble compressibility term
  ADReal material_time_derivative_rho_d = u * rho_d_grad(0);
  if (_dim > 1)
  {
    material_time_derivative_rho_d += (*_v_var)(elem_arg, state) * rho_d_grad(1);
    if (_dim > 2)
    {
      material_time_derivative_rho_d += (*_w_var)(elem_arg, state) * rho_d_grad(2);
    }
  }
  if (is_transient)
    material_time_derivative_rho_d +=
        raw_value(_rho_d(elem_arg, state) - _rho_d(elem_arg, Moose::oldState())) / _dt;
  const auto bubble_compressibility = material_time_derivative_rho_d * xi / 3.0;

  // Adding area growth due to added mass
  ADReal bubble_added_mass;
  if (f_d < _cutoff_fraction)
    bubble_added_mass = raw_value(_rho_d(elem_arg, state)) *
                        (f_d * 6.0 / _particle_diameter(elem_arg, state) - _var(elem_arg, state));
  else
    bubble_added_mass = 2. / 3. * _mass_exchange_coefficient(elem_arg, state) *
                        (1.0 / (f_d + libMesh::TOLERANCE) - 2.0);

  // Model parameters
  const auto db = _shape_factor * f_d_o_xi_old + libMesh::TOLERANCE;

  ADRealVectorValue velocity(u);
  if (_v_var)
    velocity(1) = (*_v_var)(elem_arg, state);
  if (_w_var)
    velocity(2) = (*_w_var)(elem_arg, state);

  const ADReal velocity_norm = NS::computeSpeed(velocity);
  const auto pressure_gradient = raw_value(_pressure.gradient(elem_arg, state));
  const Real pressure_grad_norm =
      MooseUtils::isZero(pressure_gradient) ? 1e-42 : pressure_gradient.norm();

  const auto u_eps =
      std::pow(velocity_norm * _characheristic_length(elem_arg, state) * pressure_grad_norm / rho_m,
               1. / 3.);

  const auto interaction_prefactor =
      Utility::pow<2>(f_d_o_xi) * u_eps / (std::pow(db, 11. / 3.) / complement_fd);

  // Adding coalescence term
  const auto f_c = interaction_prefactor * _gamma_c * Utility::pow<2>(f_d);
  const auto exp_c = std::exp(-_Kc * std::pow(db, 5. / 6.) * std::sqrt(rho_l / sigma) * u_eps);
  const auto s_rc = f_c * exp_c;

  // Adding breakage term
  const auto f_b = interaction_prefactor * _gamma_b * f_d * (1. - f_d);
  const auto exp_b =
      std::exp(-_Kb * sigma / (rho_l * std::pow(db, 5. / 3.) * Utility::pow<2>(u_eps)));
  const auto s_rb = f_b * exp_b;

  return -bubble_added_mass + bubble_compressibility + s_rc - s_rb;
}
