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

#include "RelationshipManager.h"
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

class ElementPointNeighbors : public RelationshipManager
{
public:
  ElementPointNeighbors(const InputParameters & parameters);
  virtual ~ElementPointNeighbors() {}

  virtual void init() override;
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;
  virtual bool isActive() const override;
  virtual std::string getInfo() const override;

protected:
  GhostPointNeighbors _point_coupling;

  bool _is_active;
};

#endif /* ELEMENTPOINTNEIGHBORS_H */
