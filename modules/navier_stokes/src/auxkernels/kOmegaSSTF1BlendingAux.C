//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kOmegaSSTF1BlendingAux.h"

registerMooseObject("NavierStokesApp", kOmegaSSTF1BlendingAux);

InputParameters
kOmegaSSTF1BlendingAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the F1 blending function for the k-omega SST model.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKESD,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("wall_distance", "Distance to the nearest wall.");
  return params;
}

kOmegaSSTF1BlendingAux::kOmegaSSTF1BlendingAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _dim(_subproblem.mesh().dimension()),
    _k(getFunctor<ADReal>(NS::TKE)),
    _omega(getFunctor<ADReal>(NS::TKESD)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _wall_distance(getFunctor<ADReal>("wall_distance"))
{
  if (!dynamic_cast<MooseVariableFV<Real> *>(&_var))
    paramError("variable",
               "'",
               name(),
               "' is currently programmed to use finite volume machinery, so make sure that '",
               _var.name(),
               "' is a finite volume variable.");
}

Real
kOmegaSSTF1BlendingAux::computeValue()
{
  // Convenient Arguments
  const auto elem_arg = makeElemArg(_current_elem);
  const Moose::StateArg state = determineState();
  const Moose::StateArg previous_state =
      Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto k = _k(elem_arg, previous_state);
  const auto omega = _omega(elem_arg, previous_state);
  const auto omega_capped = std::max(_omega(elem_arg, previous_state).value(), 1e-12);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  const auto y = std::max(_wall_distance(elem_arg, state).value(), 1e-10);

  // Computing cross diffusion term
  const auto & grad_k = _k.gradient(elem_arg, previous_state);
  const auto & grad_omega = _omega.gradient(elem_arg, previous_state);
  auto cross_diffusion = grad_k(0) * grad_omega(0);
  if (_dim > 1)
    cross_diffusion += grad_k(1) * grad_omega(1);
  if (_dim > 2)
    cross_diffusion += grad_k(2) * grad_omega(2);
  // cross_diffusion *= 2 * rho / omega_capped / _sigma_omega_2;
  cross_diffusion *= 1 / omega_capped;
  const auto cross_diffusion_plus = std::max(cross_diffusion.value(), 1e-20);

  // Computing phi_1
  const auto T1 = std::sqrt(k) / (0.09 * omega_capped * y);
  const auto T2 = 500.0 * mu / (rho * Utility::pow<2>(y) * omega_capped);
  // const auto T3 = 4.0 * rho * k / (_sigma_omega_2 * cross_diffusion_plus * Utility::pow<2>(y));
  const auto T3 = 2.0 * k / (cross_diffusion_plus * Utility::pow<2>(y));
  const auto phi_1 = std::min(std::max(T1, T2), T3);

  return std::tanh(Utility::pow<4>(phi_1)).value();
}
