//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExodusSteadyAndAdjoint.h"
#include "DisplacedProblem.h"

registerMooseObject("OptimizationApp", ExodusSteadyAndAdjoint);

InputParameters
ExodusSteadyAndAdjoint::validParams()
{
  // Get the base class parameters
  InputParameters params = Exodus::validParams();

  // Return the InputParameters
  return params;
}

ExodusSteadyAndAdjoint::ExodusSteadyAndAdjoint(const InputParameters & parameters)
  : Exodus(parameters),
    _steady_and_adjoint_exec(dynamic_cast<SteadyAndAdjoint *>(_app.getExecutioner()))
{
}

void
ExodusSteadyAndAdjoint::incrementFileCounter()
{
  if (_steady_and_adjoint_exec)
    _file_num = _steady_and_adjoint_exec->getIterationNumberOutput();
  else if (_exodus_mesh_changed || _sequence)
    _file_num++;
}

void
ExodusSteadyAndAdjoint::outputNodalVariables()
{
  // Set the output variable to the nodal variables
  std::vector<std::string> nodal(getNodalVariableOutput().begin(), getNodalVariableOutput().end());
  _exodus_io_ptr->set_output_variables(nodal);

  // Write the data via libMesh::ExodusII_IO
  if (_discontinuous)
    _exodus_io_ptr->write_timestep_discontinuous(
        filename(),
        *_es_ptr,
        _exodus_num,
        (_steady_and_adjoint_exec ? _steady_and_adjoint_exec->getIterationNumberOutput() : time()) +
            +_app.getGlobalTimeOffset());
  else
    _exodus_io_ptr->write_timestep(
        filename(),
        *_es_ptr,
        _exodus_num,
        (_steady_and_adjoint_exec ? _steady_and_adjoint_exec->getIterationNumberOutput() : time()) +
            _app.getGlobalTimeOffset());

  if (!_overwrite)
    _exodus_num++;

  // This satisfies the initialization of the ExodusII_IO object
  handleExodusIOMeshRenumbering();
  _exodus_initialized = true;
}
