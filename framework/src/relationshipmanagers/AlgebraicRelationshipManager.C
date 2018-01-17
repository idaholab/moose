/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
