//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterMesh.h"

#include "libmesh/enum_point_locator_type.h"
#include "libmesh/int_range.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/explicit_system.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe_base.h"

using namespace libMesh;

ParameterMesh::ParameterMesh(const FEType & param_type,
                             const std::string & exodus_mesh,
                             const std::vector<std::string> & var_names)
  : _communicator(MPI_COMM_SELF),
    _mesh(_communicator),
    _param_var_id(0),
    _dof_map(nullptr),
    _fe_type(param_type)
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

  // Update cached values for gradient computations
  const_cast<unsigned short int &>(_param_var_id) = var_id;
  const_cast<const DofMap *&>(_dof_map) = &_sys->get_dof_map();
  const_cast<FEType &>(_fe_type) = _dof_map->variable_type(_param_var_id);
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<Real> & weights) const
{
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(pt);
  if (!elem)
    mooseError("No element was found to contain point ", pt);

  // Get the dof_indices for our element
  // variable id is hard coded to _param_var_id
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  _dof_map->dof_indices(elem, dof_indices, _param_var_id);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, pt);
  // get the shape function value via the FEInterface
  FEComputeData fe_data(*_eq, coor);
  FEInterface::compute_data(elem->dim(), _fe_type, elem, fe_data);
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
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(pt);

  // Get the dof_indices for our element
  // variable id is hard coded to _param_var_id
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  _dof_map->dof_indices(elem, dof_indices, _param_var_id);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, pt);
  // get the shape function value via the FEInterface
  FEComputeData fe_data(*_eq, coor);
  fe_data.enable_derivative();
  FEInterface::compute_data(elem->dim(), _fe_type, elem, fe_data);
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

Real
ParameterMesh::computeGradientL2RegularizationObjective(
    const std::vector<Real> & parameter_values) const
{
  if (parameter_values.size() != _param_dofs)
    mooseError("Parameter values size (",
               parameter_values.size(),
               ") does not match mesh DOFs (",
               _param_dofs,
               ")");

  Real total_variation = 0.0;

  // Iterate over all elements in the mesh
  for (const auto & elem : _mesh.element_ptr_range())
  {
    // Get DOF indices for this element
    std::vector<dof_id_type> dof_indices;
    _dof_map->dof_indices(elem, dof_indices, _param_var_id);

    // Get quadrature rule for this element
    const unsigned int dim = elem->dim();
    QGauss qrule(dim, _fe_type.default_quadrature_order());

    // Create finite element objects
    std::unique_ptr<FEBase> fe(FEBase::build(dim, _fe_type));
    fe->attach_quadrature_rule(&qrule);

    // Request derivatives before reinit
    const std::vector<Real> & JxW = fe->get_JxW();
    const std::vector<std::vector<RealGradient>> & dphi = fe->get_dphi();

    // Reinitialize for current element
    fe->reinit(elem);

    // Loop over quadrature points
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Compute parameter gradient at this quadrature point
      RealGradient param_grad;
      for (const auto i : index_range(dof_indices))
        param_grad += parameter_values[dof_indices[i]] * dphi[i][qp];

      // Add L2 norm squared of gradient for regularization
      total_variation += param_grad.norm_sq() * JxW[qp];
    }
  }

  return total_variation;
}

std::vector<Real>
ParameterMesh::computeGradientL2RegularizationGradient(
    const std::vector<Real> & parameter_values) const
{
  if (parameter_values.size() != _param_dofs)
    mooseError("Parameter values size (",
               parameter_values.size(),
               ") does not match mesh DOFs (",
               _param_dofs,
               ")");

  std::vector<Real> l2_gradient(_param_dofs, 0.0);

  // Iterate over all elements in the mesh
  for (const auto & elem : _mesh.element_ptr_range())
  {
    // Get DOF indices for this element
    std::vector<dof_id_type> dof_indices;
    _dof_map->dof_indices(elem, dof_indices, _param_var_id);

    // Get quadrature rule for this element
    // We recreate every time because we want the ability to combine 2d and 3d elements
    const unsigned int dim = elem->dim();
    QGauss qrule(dim, _fe_type.default_quadrature_order());

    // Create finite element objects
    std::unique_ptr<FEBase> fe(FEBase::build(dim, _fe_type));
    fe->attach_quadrature_rule(&qrule);

    // Request derivatives before reinit
    const std::vector<Real> & JxW = fe->get_JxW();
    const std::vector<std::vector<RealGradient>> & dphi = fe->get_dphi();

    // Reinitialize for current element
    fe->reinit(elem);

    // Loop over quadrature points
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Compute parameter gradient at this quadrature point
      RealGradient param_grad;
      for (const auto i : index_range(dof_indices))
        param_grad += parameter_values[dof_indices[i]] * dphi[i][qp];

      // Compute gradient contribution: 2 * grad(p) * dphi_j
      for (const auto j : index_range(dof_indices))
        l2_gradient[dof_indices[j]] += 2.0 * param_grad * dphi[j][qp] * JxW[qp];
    }
  }

  return l2_gradient;
}
