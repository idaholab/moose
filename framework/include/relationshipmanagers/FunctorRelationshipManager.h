//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RelationshipManager.h"

#include "libmesh/ghosting_functor.h"

class MooseMesh;

/**
 * Intermediate base class for RelationshipManagers that are simply built
 * using ghosting functors.  The functor should be built in internalInit()
 * and set as _functor
 */
class FunctorRelationshipManager : public RelationshipManager
{
public:
  static InputParameters validParams();

  FunctorRelationshipManager(const InputParameters & parameters);

  FunctorRelationshipManager(const FunctorRelationshipManager & other);

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  virtual void mesh_reinit() override;

  virtual void dofmap_reinit() override;

  virtual void redistribute() override;

  virtual void delete_remote_elements() override;

  /**
   * It is often called after cloning a ghosting functor/RM.
   * It is essential because the operations in a ghosting functor are mesh-dependent.
   */
  virtual void set_mesh(const MeshBase * mesh) override
  {
    if (_functor)
    {
      _functor->set_mesh(mesh);
      _mesh = mesh;
    }
    else
      mooseError("functor does not exist");
  }

protected:
  std::unique_ptr<GhostingFunctor> _functor;
};
