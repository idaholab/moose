//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarseMeshExtraElementIDGenerator.h"

registerMooseObject("ReactorApp", CoarseMeshExtraElementIDGenerator);

#include "MooseMeshUtils.h"

#include "libmesh/enum_point_locator_type.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_serializer.h"

InputParameters
CoarseMeshExtraElementIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>(
      "input", "Name of an existing mesh generator to which we assign coarse element IDs");
  params.addRequiredParam<MeshGeneratorName>(
      "coarse_mesh", "Name of an existing mesh generator as the coarse mesh");
  params.addRequiredParam<std::string>(
      "extra_element_id_name", "Name for the extra element ID that is added to the input mesh");
  params.addParam<std::string>(
      "coarse_mesh_extra_element_id",
      "Name for the extra element ID that is copied from the coarse mesh (default to element ID)");
  params.addParam<std::vector<SubdomainName>>(
      "subdomains", std::vector<SubdomainName>(), "Subdomains to apply extra element IDs to.");
  params.addParam<bool>("enforce_mesh_embedding",
                        false,
                        "True to error out when the input mesh is not embedded in the coarse mesh");

  params.addClassDescription("Assign coarse element IDs for elements on a "
                             "mesh based on a coarse mesh.");
  return params;
}

CoarseMeshExtraElementIDGenerator::CoarseMeshExtraElementIDGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _coarse_mesh(getMesh("coarse_mesh")),
    _coarse_id_name(getParam<std::string>("extra_element_id_name")),
    _using_coarse_element_id(isParamValid("coarse_mesh_extra_element_id")),
    _embedding_necessary(getParam<bool>("enforce_mesh_embedding"))
{
}

std::unique_ptr<MeshBase>
CoarseMeshExtraElementIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  unsigned int coarse_id;
  const bool already_has_id = mesh->has_elem_integer(_coarse_id_name);
  if (!already_has_id)
    coarse_id = mesh->add_elem_integer(_coarse_id_name);
  else
    coarse_id = mesh->get_elem_integer_index(_coarse_id_name);

  // Get the requested subdomain IDs
  std::set<SubdomainID> included_subdomains;
  std::set<SubdomainName> bad_subdomains;
  for (const auto & snm : getParam<std::vector<SubdomainName>>("subdomains"))
  {
    auto sid = MooseMeshUtils::getSubdomainID(snm, *mesh);
    if (!MooseMeshUtils::hasSubdomainID(*mesh, sid))
      bad_subdomains.insert(snm);
    else
      included_subdomains.insert(sid);
  }
  if (!bad_subdomains.empty())
    paramError("subdomains",
               "The requested subdomains do not exist on the fine mesh: ",
               Moose::stringify(bad_subdomains));

  // If we are not going through the entire mesh and the ID already exists, we need to offset the
  // assigned ID so that we don't copy ones that are already there
  dof_id_type id_offset = 0;
  if (already_has_id && !included_subdomains.empty())
  {
    for (auto & elem : mesh->active_element_ptr_range())
    {
      dof_id_type elem_id = elem->get_extra_integer(coarse_id);
      if (elem_id != DofObject::invalid_id && elem_id > id_offset)
        id_offset = elem_id;
    }
    id_offset++;
  }

  std::unique_ptr<MeshBase> coarse_mesh = std::move(_coarse_mesh);

  bool using_subdomain_id = false;
  unsigned int id_for_assignment = 0;
  if (_using_coarse_element_id)
  {
    const auto & id_name = getParam<std::string>("coarse_mesh_extra_element_id");
    if (id_name == "subdomain_id")
      using_subdomain_id = true;
    else
    {
      using_subdomain_id = false;
      if (!coarse_mesh->has_elem_integer(id_name))
        paramError("coarse_mesh_extra_element_id",
                   "The extra element ID does not exist on the coarse mesh");
      else
        id_for_assignment = coarse_mesh->get_elem_integer_index(id_name);
    }
  }

  // a relative tolerance on checking if fine mesh is embedded in the coarse mesh
  Real aeps = 0.01;

  // The coarse mesh is serialized for distributed mesh to avoid boundary issues.
  // As it is coarse this should be cheap. This will be a null operation for a replicated mesh.
  MeshSerializer tm(*coarse_mesh);

  // build a point_locator on coarse mesh
  std::unique_ptr<PointLocatorBase> point_locator =
      PointLocatorBase::build(TREE_ELEMENTS, *coarse_mesh);
  point_locator->enable_out_of_mesh_mode();

  // loop through fine mesh elements and get element's centroid
  auto elem_range = included_subdomains.empty()
                        ? mesh->active_element_ptr_range()
                        : mesh->active_subdomain_set_elements_ptr_range(included_subdomains);
  for (auto & elem : elem_range)
  {
    // Get the centroid of the fine elem
    Point centroid = elem->true_centroid();

    // Find coarse elem
    const Elem * coarse_elem = (*point_locator)(centroid);
    if (!coarse_elem)
      mooseError("Could not find a coarse element containing a fine element with centroid ",
                 centroid);

    // get id from the coarse element
    dof_id_type elem_id;
    if (_using_coarse_element_id)
    {
      if (using_subdomain_id)
        elem_id = coarse_elem->subdomain_id();
      else
        elem_id = coarse_elem->get_extra_integer(id_for_assignment);
    }
    else
      elem_id = coarse_elem->id();
    elem_id += id_offset;

    // Check if the fine elem is nested in the coarse element
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
    {
      // Get the node: we need to manually move it towards the centroid to
      // ensure that nothing weird happes due to round-off
      Point current_node = elem->point(n);
      current_node.add_scaled(current_node, -aeps);
      current_node.add_scaled(centroid, aeps);

      // Get the element this node is in and check if it is the same
      // as the coarse elem; however check if node_elem is valid as it
      // might not be in case the sub element it outside the coarse domain
      const Elem * node_elem = (*point_locator)(current_node);
      if (!node_elem)
        mooseError("Could not find a coarse element containing a node of fine element at ",
                   elem->point(n));

      // get another id from the coarse element with this node
      dof_id_type node_elem_id;
      if (_using_coarse_element_id)
      {
        if (using_subdomain_id)
          node_elem_id = node_elem->subdomain_id();
        else
          node_elem_id = node_elem->get_extra_integer(id_for_assignment);
      }
      else
        node_elem_id = node_elem->id();

      if (node_elem_id != elem_id)
        if (_embedding_necessary)
          mooseError(
              "Input mesh is not nested in the coarse mesh in CoarseMeshExtraElementIDGenerator.");
    }

    elem->set_extra_integer(coarse_id, elem_id);
  }

  return mesh;
}
