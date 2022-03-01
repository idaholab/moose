//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraNodesetGenerator.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", ExtraNodesetGenerator);

InputParameters
ExtraNodesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The names of the boundaries to create");

  params.addParam<std::vector<unsigned int>>("nodes",
                                             "The nodes you want to be in the nodeset "
                                             "(Either this parameter or \"coord\" must be "
                                             "supplied).");
  params.addParam<std::vector<std::vector<Real>>>(
      "coord",
      "The nodes with coordinates you want to be in the "
      "nodeset. Separate multple coords with ';' (Either this parameter or \"nodes\" must be "
      "supplied).");
  params.addParam<Real>(
      "tolerance", TOLERANCE, "The tolerance in which two nodes are considered identical");
  params.addClassDescription(
      "Creates a new node set and a new boundary made with the nodes the user provides.");

  return params;
}

ExtraNodesetGenerator::ExtraNodesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ExtraNodesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  const auto coord = getParam<std::vector<std::vector<Real>>>("coord");
  const auto nodes = getParam<std::vector<unsigned int>>("nodes");

  // make sure the input is not empty
  bool data_valid = false;
  if (isParamValid("nodes") && nodes.size() != 0)
    data_valid = true;
  if (_pars.isParamValid("coord") && coord.size() != 0)
    data_valid = true;

  if (!data_valid)
    mooseError("Node set can not be empty!");

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, boundary_names, true);

  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // add nodes with their ids
  for (const auto & node_id : nodes)
  {
    // Our mesh may be distributed and this node may not exist on this process
    if (!mesh->query_node_ptr(node_id))
      continue;

    for (const auto & boundary_id : boundary_ids)
      boundary_info.add_node(node_id, boundary_id);
  }

  // add nodes with their coordinates
  const auto dim = mesh->mesh_dimension();

  std::unique_ptr<PointLocatorBase> locator = mesh->sub_point_locator();
  locator->enable_out_of_mesh_mode();

  const auto tolerance = getParam<Real>("tolerance");
  for (const auto & c : coord)
  {
    Point p;
    if (c.size() < dim)
      paramError("coord",
                 "Coordinate ",
                 Moose::stringify(c),
                 " does not have enough components for a ",
                 dim,
                 "D mesh.");

    if (c.size() > 3)
      paramError("coord",
                 "Coordinate ",
                 Moose::stringify(c),
                 " has too many components. Did you maybe forget to separate multiple coordinates "
                 "with a ';'?");

    for (unsigned int j = 0; j < c.size(); ++j)
      p(j) = c[j];

    // locate candidate element
    bool on_node = false;
    bool found_elem = false;
    const Elem * elem = (*locator)(p);
    if (elem)
    {
      found_elem = true;
      for (unsigned int j = 0; j < elem->n_nodes(); ++j)
      {
        const Node * node = elem->node_ptr(j);
        if (p.absolute_fuzzy_equals(*node, tolerance))
        {
          for (const auto & boundary_id : boundary_ids)
            boundary_info.add_node(node, boundary_id);

          on_node = true;
          break;
        }
      }
    }

    // If we are on a distributed mesh, then any particular processor
    // may be unable to find any particular node, but *some* processor
    // should have found it.
    if (!mesh->is_replicated())
    {
      this->comm().max(found_elem);
      this->comm().max(on_node);
    }

    if (!found_elem)
      mooseError(
          "Unable to locate the following point within the domain, please check its coordinates:\n",
          p);

    if (!on_node)
      mooseError("No node found at point:\n", p);
  }

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.nodeset_name(boundary_ids[i]) = boundary_names[i];

  return dynamic_pointer_cast<MeshBase>(mesh);
}
