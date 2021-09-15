//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"

class FunctionIsotropicEigenstrain : public ComputeEigenstrainBase
{
public:
  static InputParameters validParams();
  FunctionIsotropicEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  const Function & _function;

  MaterialProperty<RankThreeTensor> & _eigenstrain_gradient;
};
