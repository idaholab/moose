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
 * ElementSideNeighborLayers is ensure that all pointwise-connected elements in ghosted to
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
