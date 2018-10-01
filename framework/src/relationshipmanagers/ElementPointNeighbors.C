//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPointNeighbors.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", ElementPointNeighbors);

template <>
InputParameters
validParams<ElementPointNeighbors>()
{
  InputParameters params = validParams<AlgebraicRelationshipManager>();

  params.addRangeCheckedParam<unsigned short>(
      "element_point_neighbor_layers",
      1,
      "element_point_neighbor_layers>=1 & element_point_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementPointNeighbors::ElementPointNeighbors(const InputParameters & parameters)
  : AlgebraicRelationshipManager(parameters),
    _element_point_neighbor_layers(getParam<unsigned short>("element_point_neighbor_layers"))
{
}

void
ElementPointNeighbors::attachRelationshipManagersInternal(Moose::RelationshipManagerType rm_type)
{
  if (_app.isSplitMesh() || _mesh.isDistributedMesh())
  {
    _point_coupling = libmesh_make_unique<PointNeighborCoupling>();
    _point_coupling->set_n_levels(_element_point_neighbor_layers);

    if (rm_type == Moose::RelationshipManagerType::GEOMETRIC)
      attachGeometricFunctorHelper(*_point_coupling);
    else
      attachAlgebraicFunctorHelper(*_point_coupling);
  }
}

std::string
ElementPointNeighbors::getInfo() const
{
  if (_point_coupling)
  {
    std::ostringstream oss;
    std::string layers = _element_point_neighbor_layers == 1 ? "layer" : "layers";

    oss << "ElementPointNeighborLayers (" << _element_point_neighbor_layers << layers << ')';
    return oss.str();
  }
  return "";
}

bool
ElementPointNeighbors::operator==(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementPointNeighbors *>(&rhs);
  if (!rm)
    return false;
  else
    return _element_point_neighbor_layers == rm->_element_point_neighbor_layers;
}

void
ElementPointNeighbors::operator()(const MeshBase::const_element_iterator &,
                                  const MeshBase::const_element_iterator &,
                                  processor_id_type,
                                  map_type &)
{
  mooseError("Unused");
}
