/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "DebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
//#include "Actions.h"
#include "libmesh/transient_system.h"


template<>
InputParameters validParams<DebugOutput>()
{
  InputParameters params = validParams<PetscOutput>();

  // Suppress unnecessary parameters
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("output_vector_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");
  params.suppressParameter<bool>("elemental_as_nodal");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("output_input");
  params.suppressParameter<bool>("output_system_information");
  params.suppressParameter<bool>("file_base");

  // Create parameters for allowing debug outputter to be defined within the [Outputs] block
  params.addParam<bool>("show_var_residual_norms", false, "Print the residual norms of the individual solution variables at each nonlinear iteration");

  return params;
}

DebugOutput::DebugOutput(const std::string & name, InputParameters & parameters) :
    PetscOutput(name, parameters),
    _show_var_residual_norms(getParam<bool>("show_var_residual_norms")),
    _sys(_problem_ptr->getNonlinearSystem().sys())
{

  // Force this outputter to output on nonlinear residuals
  _output_nonlinear = true;

  // Check the name of the object to see if was created by SetupDebugAction
  if (name.compare("_moose_debug_output") == 0)
  {
    // Extract the SetupDebugAction object
    std::vector<Action *> actions = _app.actionWarehouse().getActionsByName("setup_debug");

    /* If the 'show_var_residual_norms' is false and a SetupDebugAction object exists, use the
       parameters from the Action */
    if (!_show_var_residual_norms && actions.size() > 0)
      _show_var_residual_norms = actions[0]->getParam<bool>("show_var_residual_norms");
  }
}

DebugOutput::~DebugOutput()
{
}

void
DebugOutput::output()
{
  if (_show_var_residual_norms && onNonlinearResidual())
  {
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
      Real varResid = _sys.calculate_norm(*_sys.rhs,var_num,DISCRETE_L2);
      oss << std::setw(27-max_name_size) << " " << std::setw(max_name_size+2) //match position of overall NL residual
          << std::left << _sys.variable_name(var_num) + ":" << varResid << "\n";
    }

    Moose::out << oss.str();
    Moose::out.flush();
  }
}

std::string
DebugOutput::filename()
{
  return std::string();
}

void
DebugOutput::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for Debug output");
}

void
DebugOutput::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for Debug output");
}

void
DebugOutput::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for Debug output");
}

void
DebugOutput::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not support for Debug output");
}

void
DebugOutput::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for Debug output");
}
