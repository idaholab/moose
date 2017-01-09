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

#include "SingleMatrixPreconditioner.h"
#include "NonlinearSystem.h"
#include "MooseUtils.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/coupling_matrix.h"

template<>
InputParameters validParams<SingleMatrixPreconditioner>()
{
  InputParameters params = validParams<MoosePreconditioner>();

  params.addParam<std::vector<NonlinearVariableName> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<NonlinearVariableName> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");
  params.addParam<std::vector<NonlinearVariableName> >("coupled_groups", "List multiple space separated groups of comma separated variables. Off-diagonal jacobians will be generated for all pairs within a group.");
  params.addParam<bool>("full", false, "Set to true if you want the full set of couplings.  Simply for convenience so you don't have to set every off_diag_row and off_diag_column combination.");

  return params;
}

SingleMatrixPreconditioner::SingleMatrixPreconditioner(const InputParameters & params) :
    MoosePreconditioner(params)
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  unsigned int n_vars = nl.nVariables();

  std::unique_ptr<CouplingMatrix> cm = libmesh_make_unique<CouplingMatrix>(n_vars);
  bool full = getParam<bool>("full");

  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries from the off_diag_row and off_diag_column parameters
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for (unsigned int i = 0; i < getParam<std::vector<NonlinearVariableName> >("off_diag_row").size(); i++)
    {
      unsigned int row = nl.getVariable(0, getParam<std::vector<NonlinearVariableName> >("off_diag_row")[i]).number();
      unsigned int column = nl.getVariable(0, getParam<std::vector<NonlinearVariableName> >("off_diag_column")[i]).number();
      (*cm)(row, column) = 1;
    }

    // off-diagonal entries from the coupled_groups parameters
    std::vector<NonlinearVariableName> groups = getParam<std::vector<NonlinearVariableName> >("coupled_groups");
    for (unsigned int i = 0; i < groups.size(); ++i)
    {
      std::vector<NonlinearVariableName> vars;
      MooseUtils::tokenize<NonlinearVariableName>(groups[i], vars, 1, ",");
      for (unsigned int j = 0; j < vars.size(); ++j)
        for (unsigned int k = j+1; k < vars.size(); ++k)
        {
          unsigned int row = nl.getVariable(0, vars[j]).number();
          unsigned int column = nl.getVariable(0, vars[k]).number();
          (*cm)(row, column) = 1;
          (*cm)(column, row) = 1;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i,j) = 1;
  }

  _fe_problem.setCouplingMatrix(std::move(cm));
}
