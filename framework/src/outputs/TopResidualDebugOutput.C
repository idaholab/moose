//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "TopResidualDebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"
#include "Console.h"
#include "Action.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"

#include "libmesh/transient_system.h"
#include "libmesh/fe_type.h"

registerMooseObject("MooseApp", TopResidualDebugOutput);

InputParameters
TopResidualDebugOutput::validParams()
{
  InputParameters params = PetscOutput::validParams();
  params.addClassDescription("Debug output object for displaying the top contributing residuals.");

  // Create parameters for allowing debug outputter to be defined within the [Outputs] block
  params.addParam<unsigned int>(
      "num_residuals", 0, "The number of top residuals to print out (0 = no output)");

  // By default operate on both nonlinear and linear residuals
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_LINEAR, EXEC_NONLINEAR, EXEC_TIMESTEP_END};
  return params;
}

TopResidualDebugOutput::TopResidualDebugOutput(const InputParameters & parameters)
  : PetscOutput(parameters),
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
  MooseMesh & mesh = _problem_ptr->mesh();

  std::vector<TopResidualDebugOutputTopResidualData> vec;
  vec.resize(residual.local_size());

  unsigned int j = 0;

  // Loop over all nodal variables
  for (const auto & node : as_range(mesh.localNodesBegin(), mesh.localNodesEnd()))
  {
    dof_id_type nd = node->id();

    for (unsigned int var = 0; var < node->n_vars(_sys.number()); ++var)
      // check that variable exists on node
      if (node->n_dofs(_sys.number(), var) > 0)
      {
        const auto & subdomain_ids = mesh.getNodeBlockIds(*node);
        dof_id_type dof_idx = node->dof_number(_sys.number(), var, 0);
        vec[j] = TopResidualDebugOutputTopResidualData(
            var, subdomain_ids, nd, *node, residual(dof_idx), false, true);
        j++;
      }
  }

  // Loop over all elemental variables
  for (const auto & elem : as_range(mesh.activeLocalElementsBegin(), mesh.activeLocalElementsEnd()))
  {
    dof_id_type elem_id = elem->id();
    const SubdomainID subdomain_id = elem->subdomain_id();

    for (unsigned int var = 0; var < elem->n_vars(_sys.number()); ++var)
      // check that variable exists on element
      if (elem->n_dofs(_sys.number(), var) > 0)
      {
        dof_id_type dof_idx = elem->dof_number(_sys.number(), var, 0);
        vec[j] = TopResidualDebugOutputTopResidualData(
            var, {subdomain_id}, elem_id, elem->vertex_average(), residual(dof_idx), false, false);
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
          vec[j] =
              TopResidualDebugOutputTopResidualData(var_num, {}, 0, Point(), residual(dof), true);
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
    {
      // Create subdomain list string for node
      const unsigned int n_subdomains = vec[i]._subdomain_ids.size();
      std::vector<SubdomainName> subdomain_names(n_subdomains);
      unsigned int i_block = 0;
      for (const auto & subdomain_id : vec[i]._subdomain_ids)
      {
        subdomain_names[i_block] = mesh.getSubdomainName(subdomain_id);
        i_block++;
      }
      const std::string subdomains_string = Moose::stringify(subdomain_names, ", ", "'", true);

      const std::string elem_or_node_string = vec[i]._is_nodal ? "node" : "element";

      Moose::err << "in subdomain(s) " << subdomains_string << " at " << elem_or_node_string << " "
                 << vec[i]._id << ": " << vec[i]._point << '\n';
    }
  }

  Moose::err << std::flush;
}
