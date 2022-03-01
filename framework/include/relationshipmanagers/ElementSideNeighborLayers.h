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

namespace libMesh
{
class GhostingFunctor;
}

/**
 * ElementSideNeighborLayers is used to increase the halo or stencil depth of each processor's
 * partition. It is useful when non-local element resources are needed when using DistributedMesh.
 */
class ElementSideNeighborLayers : public FunctorRelationshipManager
{
public:
  static InputParameters validParams();

  ElementSideNeighborLayers(const InputParameters & parameters);

  ElementSideNeighborLayers(const ElementSideNeighborLayers & other);

  /**
   * According to the base class docs, "We call mesh_reinit() whenever
   * the relevant Mesh has changed, but before remote elements on a
   * distributed mesh are deleted."
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual std::string getInfo() const override;
  virtual bool operator>=(const RelationshipManager & rhs) const override;

  void dofmap_reinit() override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  /// Size of the halo or stencil of elements available in each local processors partition. Only
  /// applicable and necessary when using DistributedMesh.
  unsigned short _layers;

  const bool _use_point_neighbors;

private:
  /**
   * Helper for initing
   */
  template <typename Functor>
  void initFunctor(Functor & functor);
};
