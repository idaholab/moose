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

#include "RelationshipManager.h"
#include "InputParameters.h"

#include "libmesh/default_coupling.h"

// Forward declarations
class ElementSideNeighborLayers;
namespace libMesh
{
class GhostingFunctor;
}

template <>
InputParameters validParams<ElementSideNeighborLayers>();

class ElementSideNeighborLayers : public RelationshipManager
{
public:
  ElementSideNeighborLayers(const InputParameters & parameters);
  virtual ~ElementSideNeighborLayers() {}

  virtual void init() override;
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;
  virtual bool isActive() const override;
  virtual std::string getInfo() const override;

protected:
  const unsigned short _element_side_neighbor_layers;

  DefaultCoupling _default_coupling;

  bool _is_active;
};

#endif /* ELEMENTSIDENEIGHBORLAYERS_H */
