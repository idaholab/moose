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

#include "ElementPointNeighbors.h"
#include "MooseMesh.h"
#include "Conversion.h"

template <>
InputParameters
validParams<ElementPointNeighbors>()
{
  InputParameters params = validParams<RelationshipManager>();

  params.set<Moose::RelationshipManagerType>("RelationshipManagerType") = Moose::Geometric;

  return params;
}

ElementPointNeighbors::ElementPointNeighbors(const InputParameters & parameters)
  : RelationshipManager(parameters), _point_coupling(_mesh), _is_active(false)
{
}

void
ElementPointNeighbors::init()
{
  _is_active = true;
}

bool
ElementPointNeighbors::isActive() const
{
  return _is_active;
}

std::string
ElementPointNeighbors::getInfo() const
{
  std::ostringstream oss;
  oss << "ElementPointNeighbors";
  return oss.str();
}

void
ElementPointNeighbors::operator()(const MeshBase::const_element_iterator & range_begin,
                                  const MeshBase::const_element_iterator & range_end,
                                  processor_id_type p,
                                  map_type & coupled_elements)
{
  _point_coupling(range_begin, range_end, p, coupled_elements);
}
