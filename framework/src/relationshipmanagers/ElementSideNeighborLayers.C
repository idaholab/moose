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
  InputParameters params = validParams<FunctorRelationshipManager>();

  params.addRangeCheckedParam<unsigned short>(
      "layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : FunctorRelationshipManager(parameters), _layers(getParam<unsigned short>("layers"))
{
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  std::ostringstream oss;
  std::string layers = _layers == 1 ? "layer" : "layers";

  oss << "ElementSideNeighborLayers (" << _layers << " " << layers << ')';
  return oss.str();
}

bool
ElementSideNeighborLayers::operator==(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementSideNeighborLayers *>(&rhs);
  if (!rm)
    return false;
  else
    return _layers == rm->_layers;
}

void
ElementSideNeighborLayers::internalInit()
{
  auto functor = libmesh_make_unique<DefaultCoupling>();
  functor->set_n_levels(_layers);

  _functor = std::move(functor);
}
