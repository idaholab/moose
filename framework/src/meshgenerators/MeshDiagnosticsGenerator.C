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

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to diagnose");
  params.addClassDescription("Runs a series of diagnostics on the mesh to detect potential issues "
                             "such as unsupported features");

  // Options for the output level
  MooseEnum chk_option("NO_CHECK INFO WARNING ERROR", "NO_CHECK");

  params.addParam<MooseEnum>(
      "examine_element_volumes", chk_option, "whether to examine volume of the elements");
  params.addParam<Real>("minimum_element_volumes", 1e-16, "minimum size for element volume");
  params.addParam<Real>("maximum_element_volumes", 1e16, "Maximum size for element volume");

  params.addParam<MooseEnum>("examine_element_types",
                             chk_option,
                             "whether to look for multiple element types in the same sub-domain");
  params.addParam<MooseEnum>(
      "examine_element_overlap", chk_option, "whether to find overlapping elements");
  params.addParam<MooseEnum>(
      "examine_nonplanar_sides", chk_option, "whether to check element sides are planar");
  params.addParam<MooseEnum>("examine_non_conformality",
                             chk_option,
                             "whether to examine the conformality of elements in the mesh");
  params.addParam<Real>("nonconformal_tol", 1e-8, "tolerance for element non-conformality");
  params.addParam<MooseEnum>(
      "search_for_adaptivity_nonconformality",
      chk_option,
      "whether to check for non-conformality arising from adaptive mesh refinement");
  return params;
}

MeshDiagnosticsGenerator::MeshDiagnosticsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _check_element_volumes(getParam<MooseEnum>("examine_element_volumes")),
    _min_volume(getParam<Real>("minimum_element_volumes")),
    _max_volume(getParam<Real>("maximum_element_volumes")),
    _check_element_types(getParam<MooseEnum>("examine_element_types")),
    _check_element_overlap(getParam<MooseEnum>("examine_element_overlap")),
    _check_non_planar_sides(getParam<MooseEnum>("examine_nonplanar_sides")),
    _check_non_conformal_mesh(getParam<MooseEnum>("examine_non_conformality")),
    _non_conformality_tol(getParam<Real>("nonconformal_tol")),
    _check_adaptivity_non_conformality(getParam<MooseEnum>("search_for_adaptivity_nonconformality"))
{
  // Check that no secondary parameters have been passed with the main check disabled
  if ((isParamSetByUser("minimum_element_volumes") ||
       isParamSetByUser("maximum_element_volumes")) &&
      _check_element_volumes == "NO_CHECK")
    paramError("examine_element_volumes",
               "You must set this parameter to true to trigger element size checks");
  if (isParamSetByUser("nonconformal_tol") && _check_non_conformal_mesh == "NO_CHECK")
    paramError("examine_non_conformality",
               "You must set this parameter to true to trigger mesh conformality check");
}

std::unique_ptr<MeshBase>
MeshDiagnosticsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Most of the checks assume we use a replicated mesh
  if (!mesh->is_replicated())
    mooseError("Only replicated meshes are supported");

  // We prepare for use at the beginning to facilitate diagnosis
  // This deliberately does not trust the mesh to know whether it's already prepared or not
  mesh->prepare_for_use();

  if (_check_element_volumes != "NO_CHECK")
  {
    // loop elements within the mesh (assumes replicated)
    for (auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->volume() <= _min_volume)
      {
        if (_num_tiny_elems < 10)
          _console << "Element too small detected with centroid : " << elem->true_centroid()
                   << std::endl;
        else if (_num_tiny_elems == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
        _num_tiny_elems++;
      }
      if (elem->volume() >= _max_volume)
      {
        if (_num_big_elems < 10)
          _console << "Element too large detected with centroid : " << elem->true_centroid()
                   << std::endl;
        else if (_num_big_elems == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
        _num_big_elems++;
      }
    }
    diagnosticsLog("Number of elements below prescribed volume : " +
                       std::to_string(_num_tiny_elems),
                   _check_element_volumes,
                   _num_tiny_elems);
    diagnosticsLog("Number of elements above prescribed volume : " + std::to_string(_num_big_elems),
                   _check_element_volumes,
                   _num_big_elems);
  }

  if (_check_element_types != "NO_CHECK")
  {
    std::set<subdomain_id_type> ids;
    mesh->subdomain_ids(ids);
    // loop on sub-domain
    for (auto & id : ids)
    {
      // ElemType defines an enum for geometric element types
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
        diagnosticsLog("Two different element types in subdomain " + std::to_string(id),
                       _check_element_types,
                       true);
    }
  }

  if (_check_element_overlap != "NO_CHECK")
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes, assumed replicated mesh
    for (auto & node : mesh->node_ptr_range())
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
          if (_num_elem_overlaps < 10)
            _console << "Element overlap detected at : " << *node << std::endl;
          else if (_num_elem_overlaps == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }

    diagnosticsLog("Number of elements overlapping (node-based heuristics): " +
                       Moose::stringify(_num_elem_overlaps),
                   _check_element_overlap,
                   _num_elem_overlaps);
    _num_elem_overlaps = 0;

    // loop on all elements in mesh: assumes a replicated mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      // find all the elements around the centroid of this element
      std::set<const Elem *> overlaps;
      (*pl)(elem->vertex_average(), overlaps);

      if (overlaps.size() > 1)
      {
        _num_elem_overlaps++;
        if (_num_big_elems < 10)
          _console << "Element overlap detected at a centroid : " << elem->vertex_average()
                   << std::endl;
        else if (_num_elem_overlaps == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
      }
    }
    diagnosticsLog("Number of elements overlapping (centroid-based heuristics): " +
                       Moose::stringify(_num_elem_overlaps),
                   _check_element_overlap,
                   _num_elem_overlaps);
  }

  if (_check_non_planar_sides != "NO_CHECK")
  {
    // loop on all elements in mesh: assumes a replicated mesh
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
        // First vector of the base
        const RealVectorValue v1 = *nodes[0] - *nodes[1];

        // Find another node so that we can form a basis. It should just be node 0, 1, 2
        // to form two independent vectors, but degenerate elements can make them aligned
        bool aligned = true;
        unsigned int third_node_index = 2;
        RealVectorValue v2;
        while (aligned && third_node_index < nodes.size())
        {
          v2 = *nodes[0] - *nodes[third_node_index++];
          aligned = MooseUtils::absoluteFuzzyEqual(v1 * v2 - v1.norm() * v2.norm(), 0);
        }

        // Degenerate element, could not find a third node that is not aligned
        if (aligned)
          continue;

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
          if (_sides_non_planar < 10)
            _console << "Nonplanar side detected at :" << elem->side_ptr(i)->vertex_average()
                     << std::endl;
          else if (_sides_non_planar == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }
    diagnosticsLog("Number of non-planar element sides detected: " +
                       Moose::stringify(_sides_non_planar),
                   _check_non_planar_sides,
                   _sides_non_planar);
  }

  if (_check_non_conformal_mesh != "NO_CHECK")
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes, assumes a replicated mesh
    for (auto & node : mesh->node_ptr_range())
    {
      pl->set_close_to_point_tol(_non_conformality_tol);
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      // loop through the set of elements near this node
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
          if (_num_nonconformal_nodes < 10)
            _console << "Non-conformality detected at  : " << *node << std::endl;
          else if (_num_nonconformal_nodes == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }
    diagnosticsLog("Number of non-conformal nodes: " + Moose::stringify(_num_nonconformal_nodes),
                   _check_non_conformal_mesh,
                   _num_nonconformal_nodes);
    pl->unset_close_to_point_tol();
  }

  if (_check_adaptivity_non_conformality != "NO_CHECK")
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes, assumes a replicated mesh
    for (auto & node : mesh->node_ptr_range())
    {
      pl->set_close_to_point_tol(_non_conformality_tol);
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      // loop through the set of elements near this node
      for (auto & elem : elements)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool node_on_elem = false;

        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            node_on_elem = true;
            break;
          }
        }
        if (node_on_elem)
          elements.erase(elem);
      }
      if (elements.size() > 0)
      {
        for (auto & elem : elements())
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

void
MeshDiagnosticsGenerator::diagnosticsLog(std::string msg,
                                         const MooseEnum & log_level,
                                         bool may_error)
{
  mooseAssert(log_level != "NO_CHECK",
              "We should not be outputting logs if the check had been disabled");
  if (log_level == "INFO" || !may_error)
    mooseInfoRepeated(msg);
  else if (log_level == "WARNING")
    mooseWarning(msg);
  else if (log_level == "ERROR")
    mooseError(msg);
  else
    mooseError("Should not reach here");
}
