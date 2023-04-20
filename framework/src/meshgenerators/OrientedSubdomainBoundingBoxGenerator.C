//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientedSubdomainBoundingBoxGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", OrientedSubdomainBoundingBoxGenerator);

InputParameters
OrientedSubdomainBoundingBoxGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += OrientedBoxInterface::validParams();

  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<subdomain_id_type>(
      "block_id", "Subdomain id to set for inside/outside the bounding box");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");
  params.addClassDescription(
      "Defines a subdomain inside or outside of a bounding box with arbitrary orientation.");

  return params;
}

OrientedSubdomainBoundingBoxGenerator::OrientedSubdomainBoundingBoxGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    OrientedBoxInterface(parameters),
    _input(getMesh("input")),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<subdomain_id_type>("block_id"))
{
}

std::unique_ptr<MeshBase>
OrientedSubdomainBoundingBoxGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Loop over the elements
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    bool contains = containsPoint(elem->vertex_average());
    if (contains && _location == "INSIDE")
      elem->subdomain_id() = _block_id;
    else if (!contains && _location == "OUTSIDE")
      elem->subdomain_id() = _block_id;
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
