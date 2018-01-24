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

#include "GeometricRelationshipManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<GeometricRelationshipManager>()
{
  InputParameters params = validParams<RelationshipManager>();

  params.set<Moose::RelationshipManagerType>("rm_type") = Moose::RelationshipManagerType::Geometric;
  return params;
}

GeometricRelationshipManager::GeometricRelationshipManager(const InputParameters & parameters)
  : RelationshipManager(parameters)
{
}

void
GeometricRelationshipManager::attachGeometricFunctorHelper(GhostingFunctor & gf) const
{
  _mesh.getMesh().add_ghosting_functor(gf);
}
