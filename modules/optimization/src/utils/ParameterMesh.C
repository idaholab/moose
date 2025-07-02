//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegment.h"
#include "MooseError.h"
#include "ParameterMesh.h"

#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/dof_map.h"

#include "libmesh/elem.h"
#include "libmesh/face_quad.h"
#include "libmesh/face_quad4.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"
#include "libmesh/id_types.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/explicit_system.h"
#include "libmesh/plane.h"
#include "libmesh/enum_to_string.h"
#include <memory>

using namespace libMesh;

ParameterMesh::ParameterMesh(const FEType & param_type,
                             const std::string & exodus_mesh,
                             const std::vector<std::string> & var_names,
                             const bool find_closest,
                             const unsigned int kdtree_candidates)
  : _communicator(MPI_COMM_SELF),
    _mesh(_communicator),
    _find_closest(find_closest),
    _kdtree_candidates(kdtree_candidates)
{
  _mesh.allow_renumbering(false);
  _mesh.prepare_for_use();
  _exodusII_io = std::make_unique<ExodusII_IO>(_mesh);
  _exodusII_io->read(exodus_mesh);
  _mesh.read(exodus_mesh);
  // Create system to store parameter values
  _eq = std::make_unique<EquationSystems>(_mesh);
  _sys = &_eq->add_system<ExplicitSystem>("_parameter_mesh_sys");
  _sys->add_variable("_parameter_mesh_var", param_type);

  // Create point locator
  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh);
  _point_locator->enable_out_of_mesh_mode();

  if (!var_names.empty())
  {
    // Make Exodus vars for equation system
    const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
    const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());

    std::vector<std::string> nodal_variables;
    std::vector<std::string> elemental_variables;
    for (const auto & var_name : var_names)
    {
      if (std::find(all_nodal.begin(), all_nodal.end(), var_name) != all_nodal.end())
        nodal_variables.push_back(var_name);
      if (std::find(all_elemental.begin(), all_elemental.end(), var_name) != all_elemental.end())
        elemental_variables.push_back(var_name);
    }
    if ((elemental_variables.size() + nodal_variables.size()) != var_names.size())
    {
      std::string out("\n  Parameter Group Variables Requested: ");
      for (const auto & var : var_names)
        out += var + " ";
      out += "\n  Exodus Nodal Variables: ";
      for (const auto & var : all_nodal)
        out += var + " ";
      out += "\n  Exodus Elemental Variables: ";
      for (const auto & var : all_elemental)
        out += var + " ";
      mooseError("Exodus file did not contain all of the parameter names being intitialized.", out);
    }

    if (!nodal_variables.empty() && !elemental_variables.empty())
    {
      std::string out("\n  Parameter Group Nodal Variables: ");
      for (const auto & var : nodal_variables)
        out += var + " ";
      out += "\n  Parameter Group Elemental Variables: ";
      for (const auto & var : elemental_variables)
        out += var + " ";
      mooseError("Parameter group contains nodal and elemental variables, this is "
                 "not allowed.  ",
                 out);
    }
    // Add the parameter group ics and bounds to the system
    // All parameters in a group will be the same type, only one of these loops will do anything
    for (const auto & var_name : nodal_variables)
      _sys->add_variable(var_name, param_type);
    for (const auto & var_name : elemental_variables)
      _sys->add_variable(var_name, param_type);
  }
  // Initialize the equations systems
  _eq->init();

  const unsigned short int var_id = _sys->variable_number("_parameter_mesh_var");
  std::set<dof_id_type> var_indices;
  _sys->local_dof_indices(var_id, var_indices);
  _param_dofs = var_indices.size();

  if (_find_closest)
  {
    for (const auto & elem : _mesh.element_ptr_range())
      if (elem->default_order() != FIRST)
        mooseError("Closet point projection currently does not support second order elements.");
  }

  // Initialize node-based KDTree optimization
  _mesh_nodes.clear();
  _node_to_elements.clear();

  // Extract all node coordinates
  for (const auto & node : _mesh.node_ptr_range())
    _mesh_nodes.push_back(*node);

  // Build node-to-elements connectivity map
  for (const auto & elem : _mesh.element_ptr_range())
  {
    for (const auto n : make_range(elem->n_nodes()))
    {
      dof_id_type node_id = elem->node_id(n);
      _node_to_elements[node_id].insert(elem);
    }
  }

  // Create KDTree from node coordinates
  if (!_mesh_nodes.empty())
    _node_kdtree = std::make_unique<KDTree>(_mesh_nodes, 10);
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<Real> & weights) const
{
  Point test_point = (_find_closest ? projectToMesh(pt) : pt);

  const Elem * elem = (*_point_locator)(test_point);
  if (!elem)
    mooseError("No element was found to contain point ", test_point);

  // Get the  in the dof_indices for our element
  // variable name is hard coded to _parameter_mesh_var
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  const unsigned short int var = _sys->variable_number("_parameter_mesh_var");
  const DofMap & dof_map = _sys->get_dof_map();
  dof_map.dof_indices(elem, dof_indices, var);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, test_point);
  // get the shape function value via the FEInterface
  FEType fe_type = dof_map.variable_type(var);
  FEComputeData fe_data(*_eq, coor);
  FEInterface::compute_data(elem->dim(), fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.shape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<RealGradient> & weights) const
{
  if (!_sys->has_variable("_parameter_mesh_var"))
    mooseError("Internal error: System being read does not contain _parameter_mesh_var.");
  Point test_point = (_find_closest ? projectToMesh(pt) : pt);
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(test_point);

  // Get the  in the dof_indices for our element
  // variable name is hard coded to _parameter_mesh_var
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  const unsigned short int var = _sys->variable_number("_parameter_mesh_var");
  const DofMap & dof_map = _sys->get_dof_map();
  dof_map.dof_indices(elem, dof_indices, var);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, test_point);
  // get the shape function value via the FEInterface
  FEType fe_type = dof_map.variable_type(var);
  FEComputeData fe_data(*_eq, coor);
  fe_data.enable_derivative();
  FEInterface::compute_data(elem->dim(), fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.dshape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}

std::vector<Real>
ParameterMesh::getParameterValues(std::string var_name, unsigned int time_step) const
{
  if (!_sys->has_variable(var_name))
    mooseError("Exodus file being read does not contain ", var_name, ".");
  // get the exodus variable and put it into the equation system.
  unsigned int step_to_read = _exodusII_io->get_num_time_steps();
  if (time_step <= step_to_read)
    step_to_read = time_step;
  else if (time_step != std::numeric_limits<unsigned int>::max())
    mooseError("Invalid value passed as \"time_step\". Expected a valid integer "
               "less than ",
               _exodusII_io->get_num_time_steps(),
               ", received ",
               time_step);

  // determine what kind of variable you are trying to read from mesh
  const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
  const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());
  if (std::find(all_nodal.begin(), all_nodal.end(), var_name) != all_nodal.end())
    _exodusII_io->copy_nodal_solution(*_sys, var_name, var_name, step_to_read);
  else if (std::find(all_elemental.begin(), all_elemental.end(), var_name) != all_elemental.end())
    _exodusII_io->copy_elemental_solution(*_sys, var_name, var_name, step_to_read);

  // Update the equations systems
  _sys->update();

  const unsigned short int var_id = _sys->variable_number(var_name);
  std::set<dof_id_type> var_indices;
  _sys->local_dof_indices(var_id, var_indices); // Everything is local so this is fine
  std::vector<dof_id_type> var_indices_vector(var_indices.begin(), var_indices.end());

  std::vector<Real> parameter_values;
  _sys->solution->localize(parameter_values, var_indices_vector);
  return parameter_values;
}

Point
ParameterMesh::projectToMesh(const Point & p) const
{
  // quick path: p already inside an element
  if ((*_point_locator)(p))
    return p;

  // Lambda to find closest point from elements using squared distance for efficiency
  auto findClosestElement = [&p, this](const auto & elements) -> Point
  {
    Real best_d2 = std::numeric_limits<Real>::max();
    Point best_point = p;

    for (const auto * elem : elements)
    {
      Point trial = closestPoint(*elem, p);
      Real d2 = (trial - p).norm_sq();
      if (d2 < best_d2)
      {
        best_d2 = d2;
        best_point = trial;
      }
    }

    if (best_d2 == std::numeric_limits<Real>::max())
      mooseError("project_to_mesh failed - no candidate elements.");
    return best_point;
  };

  // Use KDTree optimization if available
  if (_node_kdtree && !_mesh_nodes.empty())
  {
    // Find K nearest nodes using KDTree
    std::vector<std::size_t> nearest_node_indices;
    _node_kdtree->neighborSearch(p, _kdtree_candidates, nearest_node_indices);

    // Collect all elements connected to these nodes
    std::set<const Elem *> candidate_elements;
    for (auto node_idx : nearest_node_indices)
    {
      // Get the actual node from the mesh using the index
      if (node_idx < _mesh.n_nodes())
      {
        const Node * node = _mesh.node_ptr(node_idx);
        dof_id_type node_id = node->id();
        auto it = _node_to_elements.find(node_id);
        if (it != _node_to_elements.end())
        {
          const auto & connected_elems = it->second;
          candidate_elements.insert(connected_elems.begin(), connected_elems.end());
        }
      }
    }

    // Convert set to vector for consistent type
    std::vector<const Elem *> candidate_vector(candidate_elements.begin(),
                                               candidate_elements.end());
    return findClosestElement(candidate_vector);
  }
  else
  {
    // Fallback to original O(n) method if KDTree not available
    std::vector<const Elem *> all_elements;
    for (const auto & elem : _mesh.element_ptr_range())
      all_elements.push_back(elem);

    return findClosestElement(all_elements);
  }
}

Point
ParameterMesh::closestPoint(const Elem & elem, const Point & p) const
{
  mooseAssert(!elem.contains_point(p),
              "Points inside of elements shouldn't need to find closestPoint.");

  // Lambda to find closest point from range without storing temporary vectors
  auto findClosest = [&p](auto range, auto point_func) -> Point
  {
    Real min_distance = std::numeric_limits<Real>::max();
    Point min_point = p;

    for (const auto & item : range)
    {
      Point candidate = point_func(item);
      Real distance = (candidate - p).norm();
      if (distance < min_distance)
      {
        min_distance = distance;
        min_point = candidate;
      }
    }
    return min_point;
  };

  switch (elem.type())
  {
    case EDGE2:
    {
      LineSegment ls(*(elem.node_ptr(0)), *(elem.node_ptr(1)));
      return ls.closest_point(p);
    }

    case TRI3:
    {
      Point a = *(elem.node_ptr(0));
      Point b = *(elem.node_ptr(1));
      Point c = *(elem.node_ptr(2));
      Plane pl(a, b, c);
      Point trial = pl.closest_point(p);
      if (elem.contains_point(trial))
        return trial;

      return findClosest(make_range(elem.n_edges()),
                         [&](dof_id_type i) { return closestPoint(*elem.build_edge_ptr(i), p); });
    }
    case QUAD4:
    {
      Point a = *(elem.node_ptr(0));
      Point b = *(elem.node_ptr(1));
      Point c = *(elem.node_ptr(2));
      Point d = *(elem.node_ptr(3));
      Plane pl1(a, b, c);
      Plane pl2(b, c, d);
      Point trial1 = pl1.closest_point(p);
      Point trial2 = pl2.closest_point(p);
      if (!trial1.absolute_fuzzy_equals(trial2, TOLERANCE * TOLERANCE))
        mooseError("Quad4 element is not coplanar");

      if (elem.contains_point(trial1))
        return trial1;

      return findClosest(make_range(elem.n_edges()),
                         [&](dof_id_type i) { return closestPoint(*elem.build_edge_ptr(i), p); });
    }

    default:
    {
      if (elem.dim() == 3)
      {
        return findClosest(make_range(elem.n_sides()),
                           [&](dof_id_type i) { return closestPoint(*elem.build_side_ptr(i), p); });
      }
      else
      {
        mooseError("Unsupported element type ",
                   Utility::enum_to_string(elem.type()),
                   " for projection of parameter mesh.");
      }
    }
  }
}
