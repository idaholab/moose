//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorRelationshipManager.h"
#include "MooseApp.h"

InputParameters
FunctorRelationshipManager::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  return params;
}

FunctorRelationshipManager::FunctorRelationshipManager(const InputParameters & parameters)
  : RelationshipManager(parameters), _functor(nullptr)
{
}

FunctorRelationshipManager::FunctorRelationshipManager(const FunctorRelationshipManager & other)
  : RelationshipManager(other), _functor(other._functor ? other._functor->clone() : nullptr)
{
}

void
FunctorRelationshipManager::operator()(const MeshBase::const_element_iterator & range_begin,
                                       const MeshBase::const_element_iterator & range_end,
                                       processor_id_type p,
                                       map_type & coupled_elements)
{
  if (!_functor)
    mooseError("Functor was not initialized!");

  (*_functor)(range_begin, range_end, p, coupled_elements);
}

void
FunctorRelationshipManager::mesh_reinit()
{
  if (!_functor)
    mooseError("Functor was not initialized!");

  _functor->mesh_reinit();
}

void
FunctorRelationshipManager::dofmap_reinit()
{
  if (!_functor)
    mooseError("Functor was not initialized!");

  _functor->dofmap_reinit();
}

void
FunctorRelationshipManager::redistribute()
{
  if (!_functor)
    mooseError("Functor was not initialized!");

  _functor->redistribute();
}

void
FunctorRelationshipManager::delete_remote_elements()
{
  if (!_functor)
    mooseError("Functor was not initialized!");

  _functor->delete_remote_elements();
}
