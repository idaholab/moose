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

#include "RelationshipManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<RelationshipManager>()
{
  InputParameters params = validParams<MooseObject>();
  /**
   * This param must be set by the Developer so that MOOSE knows what type of object this
   * is before it is constructed. An alternative design was considered where we would just create
   * separate base classes and have developers register against those bases separately but we don't
   * actually store the base type in the Factory anyway so this would still require several more
   * changes and not solve every problem.
   *
   * TL;DR: When building a relationship manager you _must_ set this parameter directly in your
   * validparams() method or you will receive an error prior to this object's construction.
   */
  params.addPrivateParam<Moose::RelationshipManagerType>("RelationshipManagerType");

  // Set by MOOSE
  params.addPrivateParam<MooseMesh *>("mesh");
  params.registerBase("RelationshipManager");
  return params;
}

RelationshipManager::RelationshipManager(const InputParameters & parameters)
  : MooseObject(parameters),
    GhostingFunctor(),
    _mesh(*getCheckedPointerParam<MooseMesh *>("mesh", "Mesh is null in RelationshipManager ctor"))
{
}
