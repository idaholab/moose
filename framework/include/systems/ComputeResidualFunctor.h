//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/nonlinear_implicit_system.h"

using namespace libMesh;

class FEProblemBase;

class ComputeResidualFunctor : public NonlinearImplicitSystem::ComputeResidual
{
private:
  FEProblemBase & _fe_problem;

public:
  ComputeResidualFunctor(FEProblemBase & fe_problem);

  void residual(const NumericVector<Number> & soln,
                NumericVector<Number> & residual,
                NonlinearImplicitSystem & sys) override;
};
