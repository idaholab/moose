//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FiniteDifferencePreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"

#include "libmesh/coupling_matrix.h"

registerMooseObjectAliased("MooseApp", FiniteDifferencePreconditioner, "FDP");

InputParameters
FiniteDifferencePreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription("Finite difference preconditioner (FDP) builds a numerical Jacobian "
                             "for preconditioning, only use for testing and verification.");

  params.addParam<bool>("implicit_geometric_coupling",
                        false,
                        "Set to true if you want to add entries into the "
                        "matrix for degrees of freedom that might be coupled "
                        "by inspection of the geometric search objects.");

  MooseEnum finite_difference_type("standard coloring", "coloring");
  params.addParam<MooseEnum>("finite_difference_type",
                             finite_difference_type,
                             "standard: standard finite difference"
                             "coloring: finite difference based on coloring");

  return params;
}

FiniteDifferencePreconditioner::FiniteDifferencePreconditioner(const InputParameters & params)
  : MoosePreconditioner(params),
    _finite_difference_type(getParam<MooseEnum>("finite_difference_type"))
{
  if (n_processors() > 1)
    mooseWarning("Finite differencing to assemble the Jacobian is MUCH MUCH slower than forming "
                 "the Jacobian by hand, so don't complain about performance if you use it!");

  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  unsigned int n_vars = nl.nVariables();

  std::unique_ptr<CouplingMatrix> cm = std::make_unique<CouplingMatrix>(n_vars);

  bool full = getParam<bool>("full");

  // standard finite difference method will add off-diagonal entries
  if (_finite_difference_type == "standard")
    full = true;

  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries
    std::vector<std::vector<unsigned int>> off_diag(n_vars);
    for (const auto i : index_range(getParam<std::vector<NonlinearVariableName>>("off_diag_row")))
    {
      unsigned int row =
          nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_row")[i])
              .number();
      unsigned int column =
          nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_column")[i])
              .number();
      (*cm)(row, column) = 1;
    }

    // TODO: handle coupling entries between NL-vars and SCALAR-vars
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i, j) = 1;
  }

  _fe_problem.setCouplingMatrix(std::move(cm));

  bool implicit_geometric_coupling = getParam<bool>("implicit_geometric_coupling");

  nl.addImplicitGeometricCouplingEntriesToJacobian(implicit_geometric_coupling);

  // Set the jacobian to null so that libMesh won't override our finite differenced jacobian
  nl.useFiniteDifferencedPreconditioner(true);
}
