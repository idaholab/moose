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
#include "TopResidualDebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"
#include "Console.h"
#include "Action.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/fe_type.h"

template <>
InputParameters
validParams<TopResidualDebugOutput>()
{
  InputParameters params = validParams<BasicOutput<PetscOutput>>();

  // Create parameters for allowing debug outputter to be defined within the [Outputs] block
  params.addParam<unsigned int>(
      "num_residuals", 0, "The number of top residuals to print out (0 = no output)");

  // By default operate on both nonlinear and linear residuals
  params.set<MultiMooseEnum>("execute_on") = "linear nonlinear timestep_end";
  return params;
}

TopResidualDebugOutput::TopResidualDebugOutput(const InputParameters & parameters)
  : BasicOutput<PetscOutput>(parameters),
    _num_residuals(getParam<unsigned int>("num_residuals")),
    _sys(_problem_ptr->getNonlinearSystemBase().system())
{
}

void
TopResidualDebugOutput::output(const ExecFlagType & /*type*/)
{
  // Display the top residuals
  if (_num_residuals > 0)
    printTopResiduals(_problem_ptr->getNonlinearSystemBase().RHS(), _num_residuals);
}

void
TopResidualDebugOutput::printTopResiduals(const NumericVector<Number> & residual, unsigned int n)
{
  // Need a reference to the libMesh mesh object
  MeshBase & mesh = _problem_ptr->mesh().getMesh();

  std::vector<TopResidualDebugOutputTopResidualData> vec;
  vec.resize(residual.local_size());

  unsigned int j = 0;
  MeshBase::node_iterator it = mesh.local_nodes_begin();
  const MeshBase::node_iterator end = mesh.local_nodes_end();
  for (; it != end; ++it)
  {
    Node & node = *(*it);
    dof_id_type nd = node.id();

    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
      if (node.n_dofs(_sys.number(), var) >
          0) // this check filters scalar variables (which are clearly not a dof on every node)
      {
        dof_id_type dof_idx = node.dof_number(_sys.number(), var, 0);
        vec[j] = TopResidualDebugOutputTopResidualData(var, nd, residual(dof_idx));
        j++;
      }
  }

  // Loop over all scalar variables
  std::vector<unsigned int> var_nums;
  _sys.get_all_variable_numbers(var_nums);
  const DofMap & dof_map = _sys.get_dof_map();
  for (const auto & var_num : var_nums)
    if (_sys.variable_type(var_num).family == SCALAR)
    {
      std::vector<dof_id_type> dof_indices;
      dof_map.SCALAR_dof_indices(dof_indices, var_num);

      for (const auto & dof : dof_indices)
        if (dof >= dof_map.first_dof() && dof < dof_map.end_dof())
        {
          vec[j] = TopResidualDebugOutputTopResidualData(var_num, 0, residual(dof), true);
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
  {
    Moose::err << "[DBG][" << processor_id() << "] " << std::setprecision(15) << vec[i]._residual
               << " '" << _sys.variable_name(vec[i]._var).c_str() << "' ";
    if (vec[i]._is_scalar)
      Moose::err << "(SCALAR)\n";
    else
      Moose::err << "at node " << vec[i]._nd << '\n';
  }
}
