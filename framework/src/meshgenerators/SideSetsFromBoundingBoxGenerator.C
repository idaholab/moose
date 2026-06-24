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
#include "libmesh/remote_elem.h"

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

  const auto boundary_id_new = MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true)[0];
  if (_boundary_names[0].empty() || !MooseUtils::isDigits(_boundary_names[0]))
  {
    boundary_info.sideset_name(boundary_id_new) = _boundary_names[0];
    boundary_info.nodeset_name(boundary_id_new) = _boundary_names[0];
  }

  // Boundaries do not need to overlap
  if (!_boundary_id_overlap)
  {
    bool found_element = false;
    bool found_side_sets = false;

    // Request to compute normal vectors
    const std::vector<Point> & face_normals = _fe_face->get_normals();

    const processor_id_type my_n_proc = mesh->n_processors();
    const processor_id_type my_proc_id = mesh->processor_id();
    typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
    std::vector<vec_type> queries(my_n_proc);

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

    if (!mesh->is_serial())
    {
      const auto queries_tag = mesh->comm().get_unique_tag(),
                 replies_tag = mesh->comm().get_unique_tag();

      std::vector<Parallel::Request> side_requests(my_n_proc - 1), reply_requests(my_n_proc - 1);

      for (const auto p : make_range(my_n_proc))
      {
        if (p == my_proc_id)
          continue;

        Parallel::Request & request = side_requests[p - (p > my_proc_id)];
        mesh->comm().send(p, queries[p], request, queries_tag);
      }

      std::vector<vec_type> responses(my_n_proc - 1);
      for (const auto p : make_range(uint(1), my_n_proc))
      {
        vec_type query;

        Parallel::Status status(mesh->comm().probe(Parallel::any_source, queries_tag));
        const processor_id_type source_pid = cast_int<processor_id_type>(status.source());

        mesh->comm().receive(source_pid, query, queries_tag);

        Parallel::Request & request = reply_requests[p - 1];

        for (const auto & q : query)
        {
          const Elem * elem = mesh->elem_ptr(q.first);
          const unsigned int side = q.second;

          if (_bounding_box.contains_point(elem->vertex_average()) != inside)
            continue;

          _fe_face->reinit(elem, side);
          const Point face_normal = _fe_face->get_normals()[0];
          if (elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal))
            responses[p - 1].push_back(std::make_pair(elem->id(), side));
        }

        mesh->comm().send(source_pid, responses[p - 1], request, replies_tag);
      }

      for (processor_id_type p = 1; p != my_n_proc; ++p)
      {
        Parallel::Status status(mesh->comm().probe(Parallel::any_source, replies_tag));
        const processor_id_type source_pid = cast_int<processor_id_type>(status.source());

        vec_type response;
        mesh->comm().receive(source_pid, response, replies_tag);

        for (const auto & r : response)
          add_side(mesh->elem_ptr(r.first), r.second);
      }

      Parallel::wait(side_requests);
      Parallel::wait(reply_requests);
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
