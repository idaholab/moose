//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTPOINTNEIGHBORS_H
#define ELEMENTPOINTNEIGHBORS_H

#include "GeometricRelationshipManager.h"
#include "InputParameters.h"

#include "libmesh/ghost_point_neighbors.h"

// Forward declarations
class ElementPointNeighbors;
namespace libMesh
{
class GhostingFunctor;
}

template <>
InputParameters validParams<ElementPointNeighbors>();

/**
 * ElementSideNeighborLayers ensures that all pointwise-connected elements are ghosted to
 * every processor's partition. It is useful when non-local element resources are needed when using
 * DistributedMesh.
 */
class ElementPointNeighbors : public GeometricRelationshipManager
{
public:
  ElementPointNeighbors(const InputParameters & parameters);

  virtual void attachRelationshipManagersInternal(Moose::RelationshipManagerType rm_type) override;
  virtual std::string getInfo() const override;

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

protected:
  /// The libMesh coupling object that provides this RM's functionality.
  std::unique_ptr<GhostPointNeighbors> _point_coupling;
};

#endif /* ELEMENTPOINTNEIGHBORS_H */
