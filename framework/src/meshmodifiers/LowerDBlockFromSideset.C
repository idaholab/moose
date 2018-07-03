//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDBlockFromSideset.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMesh.h"

#include "libmesh/mesh.h"

#include <set>

registerMooseObject("MooseApp", LowerDBlockFromSideset);

template <>
InputParameters
validParams<LowerDBlockFromSideset>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<SubdomainID>("new_block_id", "The lower dimensional block id to create");
  params.addParam<SubdomainName>("new_block_name",
                                 "The lower dimensional block name to create (optional)");
  params.addRequiredParam<std::vector<BoundaryID>>(
      "sidesets", "The sidesets from which to create the new block");

  params.addClassDescription("Adds lower dimensional elements on the specified sidesets.");
  return params;
}

LowerDBlockFromSideset::LowerDBlockFromSideset(const InputParameters & parameters)
  : MeshModifier(parameters),
    _new_block_id(getParam<SubdomainID>("new_block_id")),
    _sidesets(getParam<std::vector<BoundaryID>>("sidesets"))
{
}

// Used to temporarily store information about which lower-dimensional
// sides to add and what subdomain id to use for the added sides.
struct ElemSideDouble
{
  ElemSideDouble(Elem * elem_in, unsigned short int side_in) : elem(elem_in), side(side_in) {}

  Elem * elem;
  unsigned short int side;
};

void
LowerDBlockFromSideset::modify()
{
  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  std::set<BoundaryID> sidesets(_sidesets.begin(), _sidesets.end());

  std::vector<ElemSideDouble> element_sides_on_boundary;
  for (const auto & triple : mesh.get_boundary_info().build_side_list())
    // 0 : elem; 1 : side; 2 : bc
    if (sidesets.count(std::get<2>(triple)))
      element_sides_on_boundary.push_back(
          ElemSideDouble(mesh.elem_ptr(std::get<0>(triple)), std::get<1>(triple)));

  for (const auto & element_side : element_sides_on_boundary)
  {
    Elem * elem = element_side.elem;
    unsigned int side = element_side.side;

    // Build a non-proxy element from this side.
    std::unique_ptr<Elem> side_elem(elem->build_side_ptr(side, /*proxy=*/false));

    // The side will be added with the same processor id as the parent.
    side_elem->processor_id() = elem->processor_id();

    // Add subdomain ID
    side_elem->subdomain_id() = _new_block_id;

    // Also assign the side's interior parent, so it is always
    // easy to figure out the Elem we came from.
    side_elem->set_interior_parent(elem);

    // Finally, add the lower-dimensional element to the Mesh.
    mesh.add_elem(side_elem.release());
  }

  // Assign block name, if provided
  if (isParamValid("new_block_name"))
    mesh.subdomain_name(_new_block_id) = getParam<SubdomainName>("new_block_name");

  mesh.prepare_for_use();
}
