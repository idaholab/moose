//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDiagnosticsGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", MeshDiagnosticsGenerator);

InputParameters
MeshDiagnosticsGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to          ");
  params.addClassDescription("");

  params.addParam<bool>(
      "examine_element_volumes", true, "whether to examine volume of the elements");
  params.addParam<Real>("minimum_element_volumes", 1e-16, "minimum size for element volume");
  params.addParam<Real>("maximum_element_volumes", 1e16, "Maximum size for element volume");

  params.addParam<bool>("examine_element_types",
                        true,
                        "whether to look for multiple element types in the same sub-domain");
  params.addParam<bool>("examine_element_overlap", true, "whether to find overlapping elements");
  params.addParam<bool>(
      "examine_nonplanar_sides", true, "whether to check element sides are planar");
  params.addParam<bool>("examine_non_conformality",
                        true,
                        "whether to examine the conformality of elements in the mesh");
  params.addParam<Real>("nonconformal_tol", 1e-2, "tolerance for element non-conformality");
  params.addParam<bool>("examine_adaptivity_non_conformal",
                        true,
                        "whether to check for adaptavity in non-conformal meshes");
  return params;
}

MeshDiagnosticsGenerator::MeshDiagnosticsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _check_element_volumes(getParam<bool>("examine_element_volumes")),
    _min_volume(getParam<Real>("minimum_element_volumes")),
    _max_volume(getParam<Real>("maximum_element_volumes")),
    _check_element_types(getParam<bool>("examine_element_types")),
    _check_element_overlap(getParam<bool>("examine_element_overlap")),
    _check_non_planar_sides(getParam<bool>("examine_nonplanar_sides")),
    _check_non_conformal_mesh(getParam<bool>("examine_non_conformality")),
    _non_conformality_tol(getParam<Real>("nonconformal_tol")),
    _check_adaptivity_non_conformality(getParam<bool>("examine_adaptivity_non_conformal"))
{
}

std::unique_ptr<MeshBase>
MeshDiagnosticsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  mesh->prepare_for_use();
  if (_check_element_volumes)
  {
    // loop elements within the mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->volume() <= _min_volume)
      {
        _num_tiny_elems++;
      }
      if (elem->volume() >= _max_volume)
      {
        _num_big_elems++;
      }
    }
    _console << "Number of elements below volume size : " << _num_tiny_elems << std::endl;
    _console << "Number of elements above volume size : " << _num_big_elems << std::endl;
  }

  if (_check_element_types)
  {
    std::set<subdomain_id_type> ids;
    mesh->subdomain_ids(ids);
    // loop on sub-domain
    for (auto & id : ids)
    {
      // ElemType defines an num for geometric element types
      std::set<ElemType> types;
      // loop on elements within this sub-domain
      for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
      {
        types.insert(elem->type());
      }
      std::string elem_type_names;
      for (auto & elem_type : types)
        elem_type_names += " " + Moose::stringify(elem_type);

      _console << "Element type in subdomain " + mesh->subdomain_name(id) + " (" +
                      std::to_string(id) + ") :" + elem_type_names
               << std::endl;
      if (types.size() > 1)
        mooseWarning("Two different element types in subdomain " + std::to_string(id));
    }
  }

  if (_check_element_overlap)
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes
    for (auto & node : mesh->local_node_ptr_range())
    {
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      for (auto & elem : elements)
      {
        if (!elem->contains_point(*node))
          continue;

        bool found = false;
        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          _num_elem_overlaps++;
          _console << "Element overlap detected at : " << *node << std::endl;
        }
      }
    }
    _console << "Number of elements overlapping (node-based heuristics): " << _num_elem_overlaps
             << std::endl;

    _num_elem_overlaps = 0;

    // loop all elements in mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      // find all the elements around the centroid of this element
      std::set<const Elem *> overlaps;
      (*pl)(elem->vertex_average(), overlaps);

      if (overlaps.size() > 1)
      {
        _num_elem_overlaps++;
        _console << "Element overlap detected at a centroid : " << elem->vertex_average()
                 << std::endl;
      }
    }
  }
  _console << "Number of elements overlapping (centroid-based heuristics): " << _num_elem_overlaps
           << std::endl;

  if (_check_non_planar_sides)
  {
    // loop all elements in mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      for (auto i : make_range(elem->n_sides()))
      {
        auto side = elem->side_ptr(i);
        std::vector<Point *> nodes;
        for (auto & node : side->node_ref_range())
          nodes.emplace_back(&node);

        if (nodes.size() <= 3)
          continue;
        RealVectorValue v1 = *nodes[0] - *nodes[1];
        RealVectorValue v2 = *nodes[0] - *nodes[2];
        bool aligned = MooseUtils::absoluteFuzzyEqual(v1 * v2 - v1.norm() * v2.norm(), 0);
        if (aligned)
          continue; // TODO

        bool found_non_planar = false;

        for (auto in : make_range(nodes.size() - 3))
        {
          RealVectorValue v3 = *nodes[0] - *nodes[in + 3];
          bool planar = MooseUtils::absoluteFuzzyEqual(v2.cross(v1) * v3, 0);
          if (!planar)
            found_non_planar = true;
        }

        if (found_non_planar)
        {
          _sides_non_planar++;
          _console << "Nonplanar side detected at :" << elem->side_ptr(i)->vertex_average()
                   << std::endl;
        }
      }
    }
    _console << "Number of nonplanar element sides detected: " << _sides_non_planar << std::endl;
  }

  if (_check_non_conformal_mesh)
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes
    for (auto & node : mesh->local_node_ptr_range())
    {
      pl->set_close_to_point_tol(_non_conformality_tol);
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      // loop through the set of elements
      for (auto & elem : elements)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool found_conformal = false;

        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            found_conformal = true;
            break;
          }
        }
        if (!found_conformal)
        {
          _num_nonconformal_nodes++;
          _console << "Non-conformality detected at  : " << *node << std::endl;
        }
      }
    }
    _console << "Number of non-conformal nodes: " << _num_nonconformal_nodes << std::endl;
    pl->unset_close_to_point_tol();
  }

  // if (_check_non_conformal_mesh)
  // {
  //   auto pl = mesh->sub_point_locator();
  //   // loop on nodes
  //   for (auto & node : mesh->local_node_ptr_range())
  //   {
  //     pl->set_close_to_point_tol(_non_conformality_tol);
  //     // find all the elements around this node
  //     std::set<const Elem *> elements;
  //     (*pl)(*node, elements);
  //     // loop through the set of elements
  //     for (auto & elem : elements)
  //     {
  //       // If the node is not part of this element's nodes, it is a
  //       // case of non-conformality
  //       bool found_conformal = false;

  //       for (auto & elem_node : elem->node_ref_range())
  //       {
  //         if (*node == elem_node)
  //         {
  //           found_conformal = true;
  //           break;
  //         }
  //       }
  //       if (!found_conformal)
  //         elements.erase(elem);
  //     }
  //     // if there is a non-conformality
  //     // loop on elements that actually do have the node as one of their nodes
  //     if (((mesh->mesh_dimension() == 2) && (elements.size() == 2)) ||
  //         ((mesh->mesh_dimension() == 3) && (elements.size() == 4)))
  //     {
  //       for (auto & elem : elements)
  //       {
  //         if (!MooseUtils::absoluteFuzzyEqual(elem->volume(), (*elements.begin())->volume()))
  //           break;
  //       }
  //     }
  //   }
  //   _console << "Number of non-conformal nodes: " << _num_nonconformal_nodes << std::endl;
  //   pl->unset_close_to_point_tol();
  // }

  if (_check_adaptivity_non_conformality)
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes
    for (auto & node : mesh->local_node_ptr_range())
    {
      pl->set_close_to_point_tol(_non_conformality_tol);
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      // loop through the set of elements
      for (auto & elem : elements)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool found_conformal = false;

        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            found_conformal = true;
            break;
          }
        }
        if (!found_conformal)
          elements.erase(elem);
      }
      if (elements.size() > 0)
      {
        for (auto & elem : mesh->active_element_ptr_range())
        {
          for (auto i : make_range(elem->n_sides()))
          {
            auto side = elem->side_ptr(i);
            std::vector<Point *> nodes;
            for (auto & node : side->node_ref_range())
              nodes.emplace_back(&node);
            {
              // if (*node ==)
            }
          }
        }
      }
    }
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
