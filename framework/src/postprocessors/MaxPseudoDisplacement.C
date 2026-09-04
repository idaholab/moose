//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxPseudoDisplacement.h"

#include "FEProblemBase.h"
#include "Remeshing.h"

#include <algorithm>

registerMooseObject("MooseApp", MaxPseudoDisplacement);

InputParameters
MaxPseudoDisplacement::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Return the largest magnitude of the pseudo-displacement accumulated "
                             "since the last remesh event.");
  return params;
}

MaxPseudoDisplacement::MaxPseudoDisplacement(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _max_pseudo_displacement(0)
{
}

void
MaxPseudoDisplacement::initialize()
{
  // A magnitude is non-negative, so zero is the identity of the maximum and the value a rank
  // holding no node contributes
  _max_pseudo_displacement = 0;
}

void
MaxPseudoDisplacement::execute()
{
  // Reporting zero rather than erroring keeps this postprocessor usable in an input whose
  // [Remeshing] block is absent or does not move the mesh, where d is identically zero
  if (!_fe_problem.hasRemeshing())
    return;

  const Remeshing & remeshing = _fe_problem.getRemeshing();
  if (!remeshing.meshMovementEnabled())
    return;

  // The map covers ghosted nodes as well as local ones, so a node is visited by more than one
  // rank. That is harmless for a maximum, unlike for a sum
  for (const auto & [_, d] : remeshing.pseudoDisplacement())
    _max_pseudo_displacement = std::max(_max_pseudo_displacement, d.norm());
}

void
MaxPseudoDisplacement::finalize()
{
  // Unconditional so that every rank reaches this collective, including the ranks that returned
  // early out of execute()
  _communicator.max(_max_pseudo_displacement);
}

Real
MaxPseudoDisplacement::getValue() const
{
  return _max_pseudo_displacement;
}
