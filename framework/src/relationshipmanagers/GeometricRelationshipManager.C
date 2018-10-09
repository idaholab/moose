//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricRelationshipManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<GeometricRelationshipManager>()
{
  InputParameters params = validParams<RelationshipManager>();

  params.set<Moose::RelationshipManagerType>("rm_type") = Moose::RelationshipManagerType::GEOMETRIC;
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
