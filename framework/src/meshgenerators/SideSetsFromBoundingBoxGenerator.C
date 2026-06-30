//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromBoundingBoxGenerator.h"
#include "Conversion.h"
#include "MooseTypes.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/remote_elem.h"

#include <map>
#include <typeinfo>

registerMooseObject("MooseApp", SideSetsFromBoundingBoxGenerator);

InputParameters
SideSetsFromBoundingBoxGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addClassDescription("Defines new sidesets using currently-defined sideset IDs inside or "
                             "outside of a bounding box.");

  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<BoundaryName>(
      "boundary_new", "Boundary on specified block within the bounding box to assign");
  params.addParam<bool>("boundary_id_overlap",
                        false,
                        "Set to true if boundaries need to overlap on sideset to be detected.");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");

  // TODO: Implement each of these in the generate() routine using utilities in SidesetGeneratorBase
  params.suppressParameter<bool>("fixed_normal");
  params.suppressParameter<std::vector<BoundaryName>>("new_boundary");

  return params;
}

SideSetsFromBoundingBoxGenerator::SideSetsFromBoundingBoxGenerator(
    const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    _location(parameters.get<MooseEnum>("location")),
    _bounding_box(MooseUtils::buildBoundingBox(parameters.get<RealVectorValue>("bottom_left"),
                                               parameters.get<RealVectorValue>("top_right"))),
    _boundary_id_overlap(parameters.get<bool>("boundary_id_overlap"))
{
  if (_boundary_id_overlap)
  {
    const std::vector<std::string> incompatible_params = {"normal",
                                                          "replace",
                                                          "include_only_external_sides",
                                                          "included_subdomains",
                                                          "included_neighbors"};
    for (const auto & param_name : incompatible_params)
      if (isParamSetByUser(param_name))
        paramError(param_name, "Parameter should not be used with boundary_id_overlap = true.");
  }

  _boundary_names.push_back(parameters.get<BoundaryName>("boundary_new"));
}

std::unique_ptr<MeshBase>
SideSetsFromBoundingBoxGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // construct the FE object so we can compute normals of faces
  setup(*mesh);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  boundary_info.build_node_list_from_side_list();

  const bool inside = (_location == "INSIDE");

  const auto & boundary_name = _boundary_names.front();
  const auto boundary_id_new = MooseMeshUtils::getBoundaryIDs(*mesh, {boundary_name}, true).front();
  if (boundary_name.empty() || !MooseUtils::isDigits(boundary_name))
  {
    boundary_info.sideset_name(boundary_id_new) = boundary_name;
    boundary_info.nodeset_name(boundary_id_new) = boundary_name;
  }

  // Boundaries do not need to overlap
  if (!_boundary_id_overlap)
  {
    bool found_element = false;
    bool found_side_sets = false;

    // Request to compute normal vectors
    const std::vector<Point> & face_normals = _fe_face->get_normals();

    typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
    std::map<processor_id_type, vec_type> queries;

    auto add_side = [&](const Elem * elem, const unsigned int side)
    {
      if (_replace)
        boundary_info.remove_side(elem, side);
      boundary_info.add_side(elem, side, boundary_id_new);
      found_side_sets = true;
    };

    // Loop over the elements
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      // boolean if element centroid is in bounding box
      bool contains = _bounding_box.contains_point(elem->vertex_average());

      // check if active elements are found either in or out of the bounding box, apropos "inside"
      if (contains == inside)
      {
        found_element = true;
        // loop over sides of elements within bounding box
        for (const auto side : make_range(elem->n_sides()))
        {
          _fe_face->reinit(elem, side);
          // We'll just use the normal of the first qp
          const Point face_normal = face_normals[0];

          if (_check_neighbor_subdomains && elem->neighbor_ptr(side) == remote_elem)
            queries[elem->processor_id()].push_back(std::make_pair(elem->id(), side));
          else if (elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal))
            add_side(elem, side);
        }
      }
    }

    if (!queries.empty())
    {
      typedef unsigned char response_type;
      auto gather_data = [&](processor_id_type /*pid*/,
                             const vec_type & query,
                             std::vector<response_type> & response)
      {
        response.reserve(query.size());

        for (const auto & q : query)
        {
          const Elem * elem = mesh->elem_ptr(q.first);
          const unsigned int side = q.second;

          response_type side_satisfies_requirements = false;
          if (_bounding_box.contains_point(elem->vertex_average()) == inside)
          {
            _fe_face->reinit(elem, side);
            const Point face_normal = _fe_face->get_normals()[0];
            side_satisfies_requirements =
                elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal);
          }
          response.push_back(side_satisfies_requirements);
        }
      };

      auto act_on_data = [&](processor_id_type /*pid*/,
                             const vec_type & query,
                             const std::vector<response_type> & response)
      {
        for (const auto i : index_range(query))
          if (response[i])
            add_side(mesh->elem_ptr(query[i].first), query[i].second);
      };

      const response_type * example = nullptr;
      Parallel::pull_parallel_vector_data(mesh->comm(), queries, gather_data, act_on_data, example);
    }

    comm().max(found_element);
    if (!found_element)
      mooseError("No elements found ", inside ? "within" : "outside", " the bounding box");

    comm().max(found_side_sets);
    if (!found_side_sets)
      mooseError("No side sets found on active elements within the bounding box");
  }
  // Boundaries need to overlap
  else
  {
    if (_included_boundary_ids.size() < 2)
      mooseError("boundary_id_old out of bounds: ",
                 _included_boundary_ids.size(),
                 " Must be 2 boundary inputs or more.");

    bool found_node = false;

    // Loop over the elements and assign node set id to nodes within the bounding box
    for (const auto node : mesh->active_node_ptr_range())
    {
      // check if nodes are inside of bounding box
      if (_bounding_box.contains_point(*node) == inside)
      {
        // read out boundary ids for nodes
        std::vector<boundary_id_type> node_boundary_ids;
        boundary_info.boundary_ids(node, node_boundary_ids);

        // sort boundary ids on node and sort boundary ids provided in input file
        std::sort(node_boundary_ids.begin(), node_boundary_ids.end());
        std::sort(_included_boundary_ids.begin(), _included_boundary_ids.end());

        // check if input boundary ids are all contained in the node
        // if true, write new boundary id on respective node
        if (std::includes(node_boundary_ids.begin(),
                          node_boundary_ids.end(),
                          _included_boundary_ids.begin(),
                          _included_boundary_ids.end()))
        {
          boundary_info.add_node(node, boundary_id_new);
          found_node = true;
        }
      }
    }

    comm().max(found_node);
    if (!found_node)
      mooseError("No nodes found within the bounding box");
  }

  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
