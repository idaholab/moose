//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGLowerDKernel.h"

class HFEMDiffusion : public DGLowerDKernel
{
public:
  static InputParameters validParams();

  HFEMDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeLowerDQpResidual() override;
  virtual Real computeLowerDQpJacobian(Moose::ConstraintJacobianType type) override;
};
