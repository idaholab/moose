//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "ReadExodusMeshVars.h"

#include "libmesh/dof_map.h"
#include "libmesh/explicit_system.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/numeric_vector.h"
#include <memory>

using namespace libMesh;

ReadExodusMeshVars::ReadExodusMeshVars(const FEType & param_type,
                                       const std::string & exodus_mesh,
                                       const std::string var_name)
  : _communicator(MPI_COMM_SELF), _mesh(_communicator), _var_name(var_name)
{
  _mesh.allow_renumbering(false);
  _mesh.prepare_for_use();
  _exodusII_io = std::make_unique<ExodusII_IO>(_mesh);
  _exodusII_io->read(exodus_mesh);
  _mesh.read(exodus_mesh);
  // Create system to store parameter values
  _eq = std::make_unique<EquationSystems>(_mesh);
  _sys = &_eq->add_system<ExplicitSystem>("_reading_exodus_mesh_var");

  // Make Exodus vars for equation system
  const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
  const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());

  std::string nodal_variable;
  std::string elemental_variable;

  if (std::find(all_nodal.begin(), all_nodal.end(), _var_name) != all_nodal.end())
    nodal_variable = _var_name;
  if (std::find(all_elemental.begin(), all_elemental.end(), _var_name) != all_elemental.end())
    elemental_variable = _var_name;

  // Add the parameter variable to system
  // All parameters in a group will be the same type, only one of these loops will do anything
  // If it tries to add the wrong type, will throw a libmesh error
  if (!nodal_variable.empty())
    _sys->add_variable(_var_name, param_type);
  if (!elemental_variable.empty())
    _sys->add_variable(_var_name, param_type);
  // Initialize the equations systems
  _eq->init();

  if (nodal_variable.empty() && elemental_variable.empty())
  {
    std::string out("\n  Parameter Requested: " + var_name);
    out += "\n  Exodus Nodal Variables: ";
    for (const auto & var : all_nodal)
      out += var + " ";
    out += "\n  Exodus Elemental Variables: ";
    for (const auto & var : all_elemental)
      out += var + " ";
    mooseError("Exodus file did not contain the parameter name being intitialized.", out);
  }
}

std::vector<Real>
ReadExodusMeshVars::getParameterValues(const unsigned int time_step) const
{
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

  // Store Exodus variable on the equation system.
  // This will give it the same order as other functions reading variables from the same exodus mesh
  const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
  const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());
  // determine what kind of variable you are trying to read from mesh
  if (std::find(all_nodal.begin(), all_nodal.end(), _var_name) != all_nodal.end())
    _exodusII_io->copy_nodal_solution(*_sys, _var_name, _var_name, step_to_read);
  else if (std::find(all_elemental.begin(), all_elemental.end(), _var_name) != all_elemental.end())
    _exodusII_io->copy_elemental_solution(*_sys, _var_name, _var_name, step_to_read);

  // Update the equations systems
  _sys->update();

  // Now read the Exodus variable from the equation system into a vector for the reporter
  const unsigned short int var_id = _sys->variable_number(_var_name);
  std::set<dof_id_type> var_indices;
  _sys->local_dof_indices(var_id, var_indices); // Everything is local so this is fine
  std::vector<dof_id_type> var_indices_vector(var_indices.begin(), var_indices.end());

  std::vector<Real> parameter_values;
  _sys->solution->localize(parameter_values, var_indices_vector);
  return parameter_values;
}
