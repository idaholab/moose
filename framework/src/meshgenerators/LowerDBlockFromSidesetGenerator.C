//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDBlockFromSidesetGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "CastUniquePointer.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <set>
#include <typeinfo>

registerMooseObject("MooseApp", LowerDBlockFromSidesetGenerator);

template <>
InputParameters
validParams<LowerDBlockFromSidesetGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<subdomain_id_type>("new_block_id",
                                             "The lower dimensional block id to create");
  params.addParam<SubdomainName>("new_block_name",
                                 "The lower dimensional block name to create (optional)");
  params.addRequiredParam<std::vector<boundary_id_type>>(
      "sidesets", "The sidesets from which to create the new block");

  params.addClassDescription("Adds lower dimensional elements on the specified sidesets.");

  return params;
}

LowerDBlockFromSidesetGenerator::LowerDBlockFromSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _new_block_id(getParam<subdomain_id_type>("new_block_id")),
    _sidesets(getParam<std::vector<boundary_id_type>>("sidesets"))
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

std::unique_ptr<MeshBase>
LowerDBlockFromSidesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  bool distributed = false;
  if (typeid(mesh).name() == typeid(std::unique_ptr<DistributedMesh>).name())
    distributed = true;

  auto side_list = mesh->get_boundary_info().build_side_list();
  std::sort(side_list.begin(),
            side_list.end(),
            [](std::tuple<dof_id_type, unsigned short int, boundary_id_type> a,
               std::tuple<dof_id_type, unsigned short int, boundary_id_type> b) {
              auto a_elem_id = std::get<0>(a);
              auto b_elem_id = std::get<0>(b);
              if (a_elem_id == b_elem_id)
              {
                auto a_side_id = std::get<1>(a);
                auto b_side_id = std::get<1>(b);
                if (a_side_id == b_side_id)
                  return std::get<2>(a) < std::get<2>(b);
                else
                  return a_side_id < b_side_id;
              }
              else
                return a_elem_id < b_elem_id;
            });

  std::set<boundary_id_type> sidesets(_sidesets.begin(), _sidesets.end());
  std::vector<ElemSideDouble> element_sides_on_boundary;
  for (const auto & triple : side_list)
    if (sidesets.count(std::get<2>(triple)))
      element_sides_on_boundary.push_back(
          ElemSideDouble(mesh->elem_ptr(std::get<0>(triple)), std::get<1>(triple)));

  dof_id_type max_elem_id = mesh->max_elem_id();
  mesh->comm().max(max_elem_id);
  auto max_elems_to_add = element_sides_on_boundary.size();
  mesh->comm().max(max_elems_to_add);

  for (MooseIndex(element_sides_on_boundary) i = 0; i < element_sides_on_boundary.size(); ++i)
  {
    Elem * elem = element_sides_on_boundary[i].elem;
    if (distributed && elem->processor_id() != processor_id())
      continue;

    unsigned int side = element_sides_on_boundary[i].side;

    // Build a non-proxy element from this side.
    std::unique_ptr<Elem> side_elem(elem->build_side_ptr(side, /*proxy=*/false));

    // The side will be added with the same processor id as the parent.
    side_elem->processor_id() = elem->processor_id();

    // Add subdomain ID
    side_elem->subdomain_id() = _new_block_id;

    // Also assign the side's interior parent, so it is always
    // easy to figure out the Elem we came from.
    side_elem->set_interior_parent(elem);

    // Add id for distributed
    if (distributed)
      side_elem->set_id(max_elem_id + processor_id() * max_elems_to_add + i);

    // Finally, add the lower-dimensional element to the Mesh.
    mesh->add_elem(side_elem.release());
  };

  // Assign block name, if provided
  if (isParamValid("new_block_name"))
    mesh->subdomain_name(_new_block_id) = getParam<SubdomainName>("new_block_name");

  return mesh;
}
