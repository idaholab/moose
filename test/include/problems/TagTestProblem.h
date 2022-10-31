//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

/**
 * FEProblem derived class for customization of callbacks. In this instance we only print out
 * something in the c-tor and d-tor, so we know the class was build and used properly.
 */
class TagTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  TagTestProblem(const InputParameters & params);

  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual,
                               unsigned int nl_sys_num = 0) override;
  virtual void computeJacobian(const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian,
                               unsigned int nl_sys_num = 0) override;

protected:
  std::set<std::string> vtags;

  std::set<std::string> mtags;
};
