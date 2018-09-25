//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AlgebraicRelationshipManager.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "Executioner.h"

template <>
InputParameters
validParams<AlgebraicRelationshipManager>()
{
  InputParameters params = validParams<GeometricRelationshipManager>();

  params.set<Moose::RelationshipManagerType>("rm_type") = Moose::RelationshipManagerType::ALGEBRAIC;
  return params;
}

AlgebraicRelationshipManager::AlgebraicRelationshipManager(const InputParameters & parameters)
  : GeometricRelationshipManager(parameters), LazyCoupleable(this)
{
}

void
AlgebraicRelationshipManager::attachAlgebraicFunctorHelper(GhostingFunctor & gf) const
{
  /**
   * If the user has restricted or demoted this RelationshipManager type (e.g. We only want the
   * geometric portion) - do nothing.
   */
  if (_rm_type == Moose::RelationshipManagerType::GEOMETRIC)
    return;

  auto & problem = _app.getExecutioner()->feProblem();

  problem.getNonlinearSystemBase().dofMap().add_algebraic_ghosting_functor(gf);
  problem.getAuxiliarySystem().dofMap().add_algebraic_ghosting_functor(gf);

  // We need to do the same thing for displaced problem
  if (problem.getDisplacedProblem())
  {
    problem.getDisplacedProblem()->nlSys().dofMap().add_algebraic_ghosting_functor(gf);
    problem.getDisplacedProblem()->auxSys().dofMap().add_algebraic_ghosting_functor(gf);
  }
}
