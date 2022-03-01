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

#include "libmesh/default_coupling.h"

namespace libMesh
{
class GhostingFunctor;
}

/**
 * ElementPointNeighborLayers is used to increase the halo or stencil depth of each processor's
 * partition. It is useful when non-local element resources are needed when using DistributedMesh.
 */
class ElementPointNeighborLayers : public FunctorRelationshipManager
{
public:
  static InputParameters validParams();

  ElementPointNeighborLayers(const InputParameters & parameters);

  ElementPointNeighborLayers(const ElementPointNeighborLayers & other);

  /**
   * A clone() is needed because GhostingFunctor can not be shared between
   * different meshes. The operations in  GhostingFunctor are mesh dependent.
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & rhs) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  /// Size of the halo or stencil of elements available in each local processors partition. Only
  /// applicable and necessary when using DistributedMesh.
  unsigned short _layers;
};
