//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTORRELATIONSHIPMANAGER_H
#define FUNCTORRELATIONSHIPMANAGER_H

#include "RelationshipManager.h"

#include "libmesh/ghosting_functor.h"

// Forward declarations
class FunctorRelationshipManager;
class MooseMesh;

template <>
InputParameters validParams<FunctorRelationshipManager>();

/**
 * Intermediate base class for RelationshipManagers that are simply built
 * using ghosting functors.  The functor should be built in internalInit()
 * and set as _functor
 */
class FunctorRelationshipManager : public RelationshipManager
{
public:
  FunctorRelationshipManager(const InputParameters & parameters);

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

protected:
  std::unique_ptr<GhostingFunctor> _functor;
};

#endif /* FUNCTORRELATIONSHIPMANAGER_H */
