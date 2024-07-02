//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kOmegaSSTF2BlendingAux.h"

registerMooseObject("NavierStokesApp", kOmegaSSTF2BlendingAux);

InputParameters
kOmegaSSTF2BlendingAux::validParams()
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

kOmegaSSTF2BlendingAux::kOmegaSSTF2BlendingAux(const InputParameters & parameters)
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
kOmegaSSTF2BlendingAux::computeValue()
{
  // Convenient Arguments
  const auto elem_arg = makeElemArg(_current_elem);
  const Moose::StateArg state = determineState();
  const Moose::StateArg previous_state =
      Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto k = _k(elem_arg, previous_state);
  const auto omega = _omega(elem_arg, previous_state);
  const auto omega_capped = std::max(omega.value(), 1e-12);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  const auto y = std::max(_wall_distance(elem_arg, state).value(), 1e-10);

  // Computing phi_2
  const auto T1 = 2.0 * std::sqrt(k) / (0.09 * omega_capped * y);
  const auto T2 = 500.0 * mu / (rho * Utility::pow<2>(y) * omega_capped);
  const auto phi_2 = std::max(T1, T2);

  return std::tanh(Utility::pow<2>(phi_2)).value();
}
