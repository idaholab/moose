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

template <>
InputParameters
validParams<AlgebraicRelationshipManager>()
{
  InputParameters params = validParams<GeometricRelationshipManager>();

  // Algebraic functors are also Geometric by definition. We'll OR these types together to
  // simplify logic when processing these types.
  params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::Geometric | Moose::RelationshipManagerType::Algebraic;
  return params;
}

AlgebraicRelationshipManager::AlgebraicRelationshipManager(const InputParameters & parameters)
  : GeometricRelationshipManager(parameters), LazyCoupleable(this), _problem(nullptr)
{
}

void
AlgebraicRelationshipManager::attachAlgebraicFunctorHelper(GhostingFunctor & gf) const
{
  mooseAssert(_problem, "Problem pointer is NULL");

  // TODO: Need to figure out Nonlinear versus Aux
  _problem->getNonlinearSystemBase().dofMap().add_coupling_functor(gf);
}
