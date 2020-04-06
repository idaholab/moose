//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialDerivativeTestKernelBase.h"
#include "RankTwoTensor.h"

/**
 * This kernel is used for testing derivatives of a material property.
 */
class MaterialDerivativeRankTwoTestKernel : public MaterialDerivativeTestKernelBase<RankTwoTensor>
{
public:
  static InputParameters validParams();

  MaterialDerivativeRankTwoTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const unsigned int _component_i;
  const unsigned int _component_j;
};
