//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/petsc_macro.h"
#include "FieldSplitPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseEnum.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/coupling_matrix.h"

registerMooseObjectAliased("MooseApp", FieldSplitPreconditioner, "FSP");

InputParameters
FieldSplitPreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();
  params.addClassDescription("Preconditioner designed to map onto PETSc's PCFieldSplit.");

  params.addRequiredParam<std::vector<std::string>>(
      "topsplit", "Entrance to splits, the top split will specify how splits will go.");
  // We should use full coupling Jacobian matrix by default
  params.addParam<bool>("full",
                        true,
                        "Set to true if you want the full set of couplings between variables "
                        "simply for convenience so you don't have to set every off_diag_row "
                        "and off_diag_column combination.");
  return params;
}

FieldSplitPreconditioner::FieldSplitPreconditioner(const InputParameters & parameters)
  : MoosePreconditioner(parameters),
    _top_split(getParam<std::vector<std::string>>("topsplit")),
    _nl(_fe_problem.getNonlinearSystemBase())
{
  // number of variables
  unsigned int n_vars = _nl.nVariables();
  // if we want to construct a full Jacobian?
  // it is recommended to have a full Jacobian for using
  // the fieldSplit preconditioner
  bool full = getParam<bool>("full");

  // how variables couple
  std::unique_ptr<CouplingMatrix> cm = std::make_unique<CouplingMatrix>(n_vars);
  if (!full)
  {
    const auto off_diag_rows = getParam<std::vector<NonlinearVariableName>>("off_diag_row");
    const auto off_diag_columns = getParam<std::vector<NonlinearVariableName>>("off_diag_column");

    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries
    std::vector<std::vector<unsigned int>> off_diag(n_vars);
    for (const auto i : index_range(off_diag_rows))
    {
      unsigned int row = _nl.getVariable(0, off_diag_rows[i]).number();
      unsigned int column = _nl.getVariable(0, off_diag_columns[i]).number();
      (*cm)(row, column) = 1;
    }
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i, j) = 1; // full coupling
  }
  _fe_problem.setCouplingMatrix(std::move(cm));

  // turn on a flag
  _nl.useFieldSplitPreconditioner(true);

  // set a top splitting
  _fe_problem.getNonlinearSystemBase().setDecomposition(_top_split);

  // apply prefix and store PETSc options
  _fe_problem.getNonlinearSystemBase().setupFieldDecomposition();
}
