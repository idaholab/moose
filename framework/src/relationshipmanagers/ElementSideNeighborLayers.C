//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSideNeighborLayers.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", ElementSideNeighborLayers);

template <>
InputParameters
validParams<ElementSideNeighborLayers>()
{
  InputParameters params = validParams<AlgebraicRelationshipManager>();

  params.addRangeCheckedParam<unsigned short>(
      "element_side_neighbor_layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : AlgebraicRelationshipManager(parameters),
    _element_side_neighbor_layers(getParam<unsigned short>("element_side_neighbor_layers"))
{
}

void
ElementSideNeighborLayers::attachRelationshipManagersInternal(
    Moose::RelationshipManagerType rm_type)
{
  _default_coupling = libmesh_make_unique<DefaultCoupling>();
  _default_coupling->set_n_levels(_element_side_neighbor_layers);

  // If we're trying to add geometric - and this one has been specified to be geometric
  if ((rm_type &
       Moose::RelationshipManagerType::GEOMETRIC == Moose::RelationshipManagerType::GEOMETRIC) &&
      (_rm_type &
       Moose::RelationshipManagerType::GEOMETRIC == Moose::RelationshipManagerType::GEOMETRIC))
  {
    std::cout << "ESNL: Adding Geometric" << std::endl;
    attachGeometricFunctorHelper(*_default_coupling);
  }

  // If we're trying to add algebraic - and this one has been specified to be algebraic
  if ((rm_type &
       Moose::RelationshipManagerType::ALGEBRAIC == Moose::RelationshipManagerType::ALGEBRAIC) &&
      (_rm_type &
       Moose::RelationshipManagerType::ALGEBRAIC == Moose::RelationshipManagerType::ALGEBRAIC))
  {
    std::cout << "ESNL: Adding Algebraic" << std::endl;

    attachAlgebraicFunctorHelper(*_default_coupling);
  }
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  if (_default_coupling)
  {
    std::ostringstream oss;
    std::string layers = _element_side_neighbor_layers == 1 ? "layer" : "layers";

    oss << "ElementSideNeighborLayers (" << _element_side_neighbor_layers << layers << ')';
    return oss.str();
  }
  return "";
}

bool
ElementSideNeighborLayers::operator==(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementSideNeighborLayers *>(&rhs);
  if (!rm)
    return false;
  else
    return _element_side_neighbor_layers == rm->_element_side_neighbor_layers;
}

void
ElementSideNeighborLayers::operator()(const MeshBase::const_element_iterator &,
                                      const MeshBase::const_element_iterator &,
                                      processor_id_type,
                                      map_type &)
{
  mooseError("Unused");
}
