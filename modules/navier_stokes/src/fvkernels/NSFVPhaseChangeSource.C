//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPhaseChangeSource.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPhaseChangeSource);

InputParameters
NSFVPhaseChangeSource::validParams()
{
  auto params = FVElementalKernel::validParams();
  params.addClassDescription("Computes the energy source due to solidification/melting.");
  params.addRequiredParam<MooseFunctorName>("liquid_fraction", "Liquid Fraction Functor.");
  params.addRequiredParam<MooseFunctorName>("L", "Latent heat.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The mixture density.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "The solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "The liquidus temperature.");
  return params;
}

NSFVPhaseChangeSource::NSFVPhaseChangeSource(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _liquid_fraction(getFunctor<ADReal>("liquid_fraction")),
    _L(getFunctor<ADReal>("L")),
    _rho(getFunctor<ADReal>(NS::density)),
    _T_solidus(getFunctor<ADReal>("T_solidus")),
    _T_liquidus(getFunctor<ADReal>("T_liquidus"))
{
}

ADReal
NSFVPhaseChangeSource::computeQpResidual()
{
  using namespace MetaPhysicL;

  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();

  const auto T_sol = _T_solidus(elem_arg, state);
  const auto T_liq = _T_liquidus(elem_arg, state);
  const auto T = _var(elem_arg, state);

  // This is necessary to have a bounded derivative
  // Otherwise the nonlinear solve won't converge!
  const auto fl = (T - T_sol) / (T_liq - T_sol);

  // The (6.0) comes from the integral of x*(1-x) between 0 and 1.
  const auto source_index = std::max(6.0 * fl * (1 - fl), (ADReal)0);
  const auto pre_factor = (_L(elem_arg, state) * _rho(elem_arg, state)) / (T_liq - T_sol);

  return source_index * pre_factor * _var.dot(elem_arg, state);
}
