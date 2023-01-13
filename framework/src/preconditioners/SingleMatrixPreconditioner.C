//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingleMatrixPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"

#include "libmesh/coupling_matrix.h"

registerMooseObjectAliased("MooseApp", SingleMatrixPreconditioner, "SMP");

InputParameters
SingleMatrixPreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription("Single matrix preconditioner (SMP) builds a preconditioner using "
                             "user defined off-diagonal parts of the Jacobian.");

  params.addParam<std::vector<NonlinearVariableName>>(
      "coupled_groups",
      "List multiple space separated groups of comma separated variables. "
      "Off-diagonal jacobians will be generated for all pairs within a group.");
  params.addParam<bool>(
      "trust_my_coupling",
      false,
      "Whether to trust my coupling even if the framework wants to be paranoid and create a full "
      "coupling matrix, which can happen when using global AD indexing for example.");

  return params;
}

SingleMatrixPreconditioner::SingleMatrixPreconditioner(const InputParameters & params)
  : MoosePreconditioner(params)
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  unsigned int n_vars = nl.nVariables();
  const auto & libmesh_system = nl.system();
  auto cm = std::make_unique<CouplingMatrix>(n_vars);

  if (!getParam<bool>("full"))
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; ++i)
      (*cm)(i, i) = 1;

    // off-diagonal entries from the off_diag_row and off_diag_column parameters
    for (const auto & off_diag :
         getParam<NonlinearVariableName, NonlinearVariableName>("off_diag_row", "off_diag_column"))
    {
      const auto row = libmesh_system.variable_number(off_diag.first);
      const auto column = libmesh_system.variable_number(off_diag.second);
      (*cm)(row, column) = 1;
    }

    // off-diagonal entries from the coupled_groups parameters
    const auto & all_vars = nl.getVariableNames();
    auto groups = getParam<std::vector<NonlinearVariableName>>("coupled_groups");
    for (const auto & group : groups)
    {
      std::vector<VariableName> vars;
      MooseUtils::tokenize(group, vars, 1, ",");
      try
      {
        MooseUtils::expandAllMatches(all_vars, vars);
      }
      catch (std::invalid_argument const & e)
      {
        mooseError("No variable name match found for '", e.what(), "'.");
      }

      for (const auto j : index_range(vars))
        for (unsigned int k = j + 1; k < vars.size(); ++k)
        {
          const auto row = libmesh_system.variable_number(vars[j]);
          const auto column = libmesh_system.variable_number(vars[k]);
          (*cm)(row, column) = 1;
          (*cm)(column, row) = 1;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; ++i)
      for (unsigned int j = 0; j < n_vars; ++j)
        (*cm)(i, j) = 1;
  }

  for (const auto i : make_range(_fe_problem.numNonlinearSystems()))
  {
    if (i == libMesh::cast_int<unsigned int>(_fe_problem.numNonlinearSystems() - 1))
      _fe_problem.setCouplingMatrix(std::move(cm), i);
    else
      _fe_problem.setCouplingMatrix(std::make_unique<CouplingMatrix>(*cm), i);
  }
  if (getParam<bool>("trust_my_coupling"))
    _fe_problem.trustUserCouplingMatrix();
}
