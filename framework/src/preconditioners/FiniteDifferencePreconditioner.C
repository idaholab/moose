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

#include "FiniteDifferencePreconditioner.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/coupling_matrix.h"


template<>
InputParameters validParams<FiniteDifferencePreconditioner>()
{
  InputParameters params = validParams<MoosePreconditioner>();

  params.addParam<std::vector<std::string> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<std::string> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");
  params.addParam<bool>("full", false, "Set to true if you want the full set of couplings.  Simply for convenience so you don't have to set every off_diag_row and off_diag_column combination.");
  params.addParam<bool>("implicit_geometric_coupling", false, "Set to true if you want to add entries into the matrix for degrees of freedom that might be coupled by inspection of the geometric search objects.");

  return params;
}

FiniteDifferencePreconditioner::FiniteDifferencePreconditioner(const InputParameters & params) :
    MoosePreconditioner(params)
{
  if (n_processors() > 1)
    mooseError("Can't use the Finite Difference Preconditioner in parallel yet!");

  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  unsigned int n_vars = nl.nVariables();

  std::unique_ptr<CouplingMatrix> cm = libmesh_make_unique<CouplingMatrix>(n_vars);

  bool full = getParam<bool>("full");

  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for (unsigned int i = 0; i < getParam<std::vector<std::string> >("off_diag_row").size(); i++)
    {
      unsigned int row = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_row")[i]).number();
      unsigned int column = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_column")[i]).number();
      (*cm)(row, column) = 1;
    }

    // TODO: handle coupling entries between NL-vars and SCALAR-vars
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i,j) = 1;
  }

  _fe_problem.setCouplingMatrix(std::move(cm));

  bool implicit_geometric_coupling = getParam<bool>("implicit_geometric_coupling");

  nl.addImplicitGeometricCouplingEntriesToJacobian(implicit_geometric_coupling);

  // Set the jacobian to null so that libMesh won't override our finite differenced jacobian
  nl.useFiniteDifferencedPreconditioner(true);
}
