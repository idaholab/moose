//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorRelationshipManager.h"

/**
 * MooseGhostPointNeighbors is used to increase the halo or stencil depth of each processor's
 * partition. It is useful when non-local element resources are needed when using DistributedMesh.
 * This ghosts more elements than ElementSideNeighborLayers with a layer level of 1 because it
 * includes not just side neighbors but point neighbors as well. This should only be used as
 * geometric ghosting functor because it uses a nullptr coupling matrix, e.g. if used as as a
 * ghosting functor on the dof_map it would ghost all dofs between local and ghosted elements
 */
class MooseGhostPointNeighbors : public FunctorRelationshipManager
{
public:
  static InputParameters validParams();

  MooseGhostPointNeighbors(const InputParameters & parameters);

  MooseGhostPointNeighbors(const MooseGhostPointNeighbors & other);

  /**
   * According to the base class docs, "We call mesh_reinit() whenever
   * the relevant Mesh has changed, but before remote elements on a
   * distributed mesh are deleted."
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual std::string getInfo() const override;
  virtual bool operator>=(const RelationshipManager & rhs) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase & mesh) override;
};
