//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualFunctor.h"
#include "FEProblemBase.h"

ComputeResidualFunctor::ComputeResidualFunctor(FEProblemBase & fe_problem) : _fe_problem(fe_problem)
{
}

void
ComputeResidualFunctor::residual(const NumericVector<Number> & soln,
                                 NumericVector<Number> & residual,
                                 NonlinearImplicitSystem & sys)
{
  _fe_problem.computingNonlinearResid() = true;
  _fe_problem.computeResidual(sys, soln, residual);
  _fe_problem.computingNonlinearResid() = false;
}
