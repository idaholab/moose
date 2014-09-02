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
#include "Material.h"
#include "Console.h"

// libMesh includesx
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
  params.addParam<unsigned int>("show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");

  // By default operate on linear residuals, this is to maintain the behavior of show_top_residuals
  params.set<bool>("linear_residuals") = true;

  return params;
}

DebugOutput::DebugOutput(const std::string & name, InputParameters & parameters) :
    PetscOutput(name, parameters),
    _show_top_residuals(getParam<unsigned int>("show_top_residuals")),
    _show_var_residual_norms(getParam<bool>("show_var_residual_norms")),
    _sys(_problem_ptr->getNonlinearSystem().sys())
{
  // Force this outputter to output on nonlinear residuals
  _output_nonlinear = true;
}

DebugOutput::~DebugOutput()
{
}

void
DebugOutput::output()
{
  // Show variable residual norms on Nonlinear iterations
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
      Real var_res_id = _sys.calculate_norm(*_sys.rhs,var_num,DISCRETE_L2);
      oss << std::setw(27-max_name_size) << " " << std::setw(max_name_size+2) //match position of overall NL residual
          << std::left << _sys.variable_name(var_num) + ":" << var_res_id << "\n";
    }

    _console << oss.str() << std::flush;
  }

  // Display the top residuals
  if (_show_top_residuals > 0)
    printTopResiduals(*(_sys.rhs), _show_top_residuals);
}

void
DebugOutput::printTopResiduals(const NumericVector<Number> & residual, unsigned int n)
{
  // Need a reference to the libMesh mesh object
  MeshBase & mesh = _problem_ptr->mesh().getMesh();

  std::vector<DebugOutputTopResidualData> vec;
  vec.resize(residual.local_size());

  unsigned int j = 0;
  MeshBase::node_iterator it = mesh.local_nodes_begin();
  const MeshBase::node_iterator end = mesh.local_nodes_end();
  for (; it != end; ++it)
  {
    Node & node = *(*it);
    dof_id_type nd = node.id();

    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
      if (node.n_dofs(_sys.number(), var) > 0)
      {
        dof_id_type dof_idx = node.dof_number(_sys.number(), var, 0);
        vec[j] = DebugOutputTopResidualData(var, nd, residual(dof_idx));
        j++;
      }
  }

  // Sort vec by residuals
  std::sort(vec.begin(), vec.end(), sortTopResidualData);

  // Display the residuals
  Moose::err << "[DBG][" << processor_id() << "] Max " << n << " residuals";
  if (j < n)
  {
    n = j;
    Moose::err << " (Only " << n << " available)";
  }
  Moose::err << std::endl;

  for (unsigned int i = 0; i < n; ++i)
    fprintf(stderr, "[DBG][%d]  % .15e '%s' at node %d\n", processor_id(), vec[i]._residual, _sys.variable_name(vec[i]._var).c_str(), vec[i]._nd);
}

std::string
DebugOutput::filename()
{
  return _file_base;
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
