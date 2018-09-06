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
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"

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
  // We need to atttach ghosting functor for both NonlinearSystem and AuxiliarySystem
  // since they have their own  dofMaps
  _app.getExecutioner()->feProblem().getNonlinearSystemBase().dofMap().add_algebraic_ghosting_functor(gf);
  _app.getExecutioner()->feProblem().getAuxiliarySystem().dofMap().add_algebraic_ghosting_functor(gf);
  // We need to do the same thing for displaced problem
  if (_app.getExecutioner()->feProblem().getDisplacedProblem()) {
    _app.getExecutioner()->feProblem().getDisplacedProblem()->nlSys().dofMap().add_algebraic_ghosting_functor(gf);
    _app.getExecutioner()->feProblem().getDisplacedProblem()->auxSys().dofMap().add_algebraic_ghosting_functor(gf);
  }
}
