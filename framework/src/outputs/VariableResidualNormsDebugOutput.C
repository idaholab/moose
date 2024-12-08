//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "VariableResidualNormsDebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"
#include "NonlinearSystemBase.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", VariableResidualNormsDebugOutput);

InputParameters
VariableResidualNormsDebugOutput::validParams()
{
  InputParameters params = PetscOutput::validParams();
  params.addClassDescription("Reports the residual norm for each variable.");

  // By default this outputs on every nonlinear iteration
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.addParam<NonlinearSystemName>(
      "nl_sys", "nl0", "The nonlinear system that we should output information for.");
  return params;
}

VariableResidualNormsDebugOutput::VariableResidualNormsDebugOutput(
    const InputParameters & parameters)
  : PetscOutput(parameters),
    _nl(_problem_ptr->getNonlinearSystemBase(
        _problem_ptr->nlSysNum(getParam<NonlinearSystemName>("nl_sys")))),
    _sys(_nl.system())
{
}

void
VariableResidualNormsDebugOutput::output()
{
  // Only output if the problem is solving for the relevant nonlinear system
  if (_nl.number() != _problem_ptr->currentNonlinearSystem().number())
    return;

  // Stream for outputting
  std::ostringstream oss;

  // Determine the maximum variable name size
  unsigned int max_name_size = 0;
  for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
  {
    unsigned int var_name_size = _sys.variable_name(var_num).size();
    if (var_name_size > max_name_size)
      max_name_size = var_name_size;
  }

  // Perform the output of the variable residuals
  oss << "    |residual|_2 of individual variables:\n";
  for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
  {
    Real var_res_id = _sys.calculate_norm(_nl.RHS(), var_num, libMesh::DISCRETE_L2);
    oss << std::setw(27 - max_name_size) << " "
        << std::setw(max_name_size + 2) // match position of overall NL residual
        << std::left << _sys.variable_name(var_num) + ":" << var_res_id << "\n";
  }

  _console << oss.str() << std::flush;
}
