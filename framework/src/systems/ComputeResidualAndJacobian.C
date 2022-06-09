//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualAndJacobian.h"
#include "FEProblemBase.h"

ComputeResidualAndJacobian::ComputeResidualAndJacobian(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

void
ComputeResidualAndJacobian::residual_and_jacobian(const NumericVector<Number> & u,
                                                  NumericVector<Number> * R,
                                                  SparseMatrix<Number> * J,
                                                  NonlinearImplicitSystem &)
{
  mooseAssert(R, "This should be non-null");
  mooseAssert(J, "This should be non-null");
  _fe_problem.computingNonlinearResid(true);
  _fe_problem.computeResidualAndJacobian(u, *R, *J);
  _fe_problem.computingNonlinearResid(false);
}
