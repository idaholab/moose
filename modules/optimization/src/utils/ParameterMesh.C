//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "ParameterMesh.h"

#include "KDTree.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/face_quad.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/explicit_system.h"

using namespace libMesh;

ParameterMesh::ParameterMesh(const FEType & param_type,
                             const std::string & exodus_mesh,
                             const std::vector<std::string> & var_names,
                             const bool find_closest)
  : _communicator(MPI_COMM_SELF), _mesh(_communicator), _find_closest(find_closest)
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
    std::vector<Point> pts;
    pts.reserve(_mesh.n_elem());
    for (const auto & elem : _mesh.element_ptr_range())
    {
      if (elem->default_order() != FIRST)
        mooseError("Closet point projection option cannot work with second order elements.");
      pts.push_back(elem->true_centroid());
    }
    _kd_tree = std::make_unique<KDTree>(pts, 20);
  }
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
  // use KD-tree to collect k nearest element IDs
  constexpr unsigned int k = 20;
  std::vector<dof_id_type> elem_ids;
  elem_ids.reserve(k);
  _kd_tree->neighborSearch(p, k, elem_ids);
  Real best_d2 = std::numeric_limits<Real>::max();
  Point best_x = p;
  for (auto id : elem_ids)
  {
    const Elem * e = _mesh.elem_ptr(id);
    unsigned int dim = e->dim();
    Point xi = FEMap::inverse_map(dim, e, p);
    // clamp xi
    auto clamp = [](Real & s) { s = std::max<Real>(-1., std::min(s, 1.)); };
    switch (e->type())
    {
      case HEX8:
      case EDGE2:
      case QUAD4:
        clamp(xi(0));
        clamp(xi(1));
        if (dim == 3)
          clamp(xi(2));
        break;
      default:
        mooseError("Unsupported element type ", e->type(), " for projection of parameter mesh.");
    }
    Point x = FEMap::map(dim, e, xi);
    Real d2 = (x - p).norm_sq();
    if (d2 < best_d2)
    {
      best_d2 = d2;
      best_x = x;
    }
  }
  if (best_d2 == std::numeric_limits<Real>::max())
    mooseError("project_to_mesh failed - no candidate elements.");
  return best_x;
}
