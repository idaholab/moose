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

#ifndef ELEMENTSIDENEIGHBORLAYERS_H
#define ELEMENTSIDENEIGHBORLAYERS_H

#include "GeometricRelationshipManager.h"

#include "libmesh/default_coupling.h"

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
class ElementSideNeighborLayers : public GeometricRelationshipManager
{
public:
  ElementSideNeighborLayers(const InputParameters & parameters);

  virtual void attachRelationshipManagersInternal(Moose::RelationshipManagerType rm_type) override;
  virtual std::string getInfo() const override;

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

protected:
  /// Size of the halo or stencil of elements available in each local processors partition. Only
  /// applicable and necessary when using DistributedMesh.
  unsigned short _element_side_neighbor_layers;

  /// The libMesh coupling object that provides this RM's functionality.
  std::unique_ptr<DefaultCoupling> _default_coupling;
};

#endif /* ELEMENTSIDENEIGHBORLAYERS_H */
