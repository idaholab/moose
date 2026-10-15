//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVVelocityJumpGreenGaussGradient.h"

#include "LinearSystem.h"
#include "MooseLinearVariableFV.h"
#include "PorousRhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", FVVelocityJumpGreenGaussGradient);

InputParameters
FVVelocityJumpGreenGaussGradient::validParams()
{
  InputParameters params = FVGreenGaussGradient::validParams();
  params.suppressParameter<MooseEnum>("limiter");
  params.addClassDescription("Computes a Green-Gauss velocity gradient using one-sided face states "
                             "on pressure baffles.");
  return params;
}

FVVelocityJumpGreenGaussGradient::FVVelocityJumpGreenGaussGradient(const InputParameters & params)
  : FVGreenGaussGradient(params)
{
}

void
FVVelocityJumpGreenGaussGradient::linkFlowSystem(const PorousRhieChowMassFlux & rc)
{
  if (_rhie_chow && _rhie_chow != &rc)
    mooseError("FVVelocityJumpGreenGaussGradient '",
               name(),
               "' is already linked to PorousRhieChowMassFlux '",
               _rhie_chow->name(),
               "'. Use a separate gradient method for each flow system.");

  _rhie_chow = &rc;
}

FVGreenGaussGradient::InternalFaceValues
FVVelocityJumpGreenGaussGradient::internalFaceValues(const FaceInfo & fi,
                                                     const ElemInfo & elem_info,
                                                     const ElemInfo & neighbor_info,
                                                     const MooseLinearVariableFV<Real> & variable,
                                                     const Real elem_value,
                                                     const Real neighbor_value) const
{
  if (!_rhie_chow || !_rhie_chow->faceIsBaffle(fi))
    return FVGreenGaussGradient::internalFaceValues(
        fi, elem_info, neighbor_info, variable, elem_value, neighbor_value);

  for (const auto component : make_range(_rhie_chow->dimension()))
  {
    const auto & velocity = _rhie_chow->velocityVariable(component);
    if (velocity.number() != variable.number() ||
        _rhie_chow->momentumSystem(component).number() != variable.sys().number())
      continue;

    // The face states are exact for a velocity that is linear on each side, so the gradient
    // carries no slope from the jump across the baffle.
    return {_rhie_chow->baffleFaceVelocity(fi, /*elem_side=*/true)(component),
            _rhie_chow->baffleFaceVelocity(fi, /*elem_side=*/false)(component)};
  }

  mooseError("FVVelocityJumpGreenGaussGradient '",
             name(),
             "' can only compute gradients of the velocity variables of PorousRhieChowMassFlux '",
             _rhie_chow->name(),
             "', but it was used for variable '",
             variable.name(),
             "'.");
}
