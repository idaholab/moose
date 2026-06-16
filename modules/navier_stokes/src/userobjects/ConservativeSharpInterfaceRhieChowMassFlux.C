//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceRhieChowMassFlux);

InputParameters
ConservativeSharpInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = ConservativeSharpInterfaceRhieChowMassFluxBase::validParams();
  params.addClassDescription(
      "Sharp-interface Rhie-Chow face-flux provider for velocity-component momentum. The primary "
      "unknowns are U components; density weighting enters through the "
      "momentum operator and mass-flux functors.");
  return params;
}

ConservativeSharpInterfaceRhieChowMassFlux::ConservativeSharpInterfaceRhieChowMassFlux(
    const InputParameters & params)
  : ConservativeSharpInterfaceRhieChowMassFluxBase(params)
{
}

void
ConservativeSharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size(),
              "Momentum component index out of range.");

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    const Real force_density =
        reducedPressureMomentumPredictorForceDensity(*elem_info, Moose::currentState())(system_i);
    rhs.add(dof_indices[0], force_density * elem_info->volume() * elem_info->coordFactor());
  }
  rhs.close();
}
