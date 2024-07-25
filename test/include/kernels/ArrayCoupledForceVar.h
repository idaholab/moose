//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayKernel.h"

/**
 * ArrayCoupledForceVar using a variable coupled through its name directly
 */
class ArrayCoupledForceVar : public ArrayKernel
{
public:
  static InputParameters validParams();

  ArrayCoupledForceVar(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;

  // Note: This is only for testing variable coupling thus full Jacobian evaluation is
  //       not implemented.

  const std::map<std::string, std::string> & _var_coef;
  std::vector<const ArrayVariableValue *> _vars;
  std::vector<RealEigenVector> _coefs;
};
