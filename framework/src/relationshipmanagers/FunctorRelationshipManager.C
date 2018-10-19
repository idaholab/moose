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

template <>
InputParameters
validParams<FunctorRelationshipManager>()
{
  InputParameters params = validParams<RelationshipManager>();
  return params;
}

FunctorRelationshipManager::FunctorRelationshipManager(const InputParameters & parameters)
  : RelationshipManager(parameters), _functor(nullptr)
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
