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

// Forward declarations
class ElementSideNeighborLayers;
namespace libMesh
{
class GhostingFunctor;
}

template <>
InputParameters validParams<ElementSideNeighborLayers>();

/**
 * ElementSideNeighborLayers is used to increase the halo or stencil depth of each processor's
 * partition. It is useful when non-local element resources are needed when using DistributedMesh.
 */
class ElementSideNeighborLayers : public FunctorRelationshipManager
{
public:
  ElementSideNeighborLayers(const InputParameters & parameters);

  virtual std::string getInfo() const override;
  virtual bool operator==(const RelationshipManager & rhs) const override;

protected:
  virtual void internalInit() override;

  /// Size of the halo or stencil of elements available in each local processors partition. Only
  /// applicable and necessary when using DistributedMesh.
  unsigned short _layers;
};

