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
#include "Executioner.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<AlgebraicRelationshipManager>()
{
  InputParameters params = validParams<GeometricRelationshipManager>();

  params.set<Moose::RelationshipManagerType>("rm_type") = Moose::RelationshipManagerType::Algebraic;
  return params;
}

AlgebraicRelationshipManager::AlgebraicRelationshipManager(const InputParameters & parameters)
  : GeometricRelationshipManager(parameters), LazyCoupleable(this)
{
}

void
AlgebraicRelationshipManager::attachAlgebraicFunctorHelper(GhostingFunctor & gf) const
{
  // TODO: Need to figure out Nonlinear versus Aux
  _app.getExecutioner()->feProblem().getNonlinearSystemBase().dofMap().add_coupling_functor(gf);
}
