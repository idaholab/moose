//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"

/**
 * This is for testing errors, it does nothing.
 */
class MatDGKernel : public DGKernel
{
public:
  static InputParameters validParams();

  MatDGKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType) override { return 0.0; }
  virtual Real computeQpJacobian(Moose::DGJacobianType) override { return 0.0; }
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType, unsigned int) override
  {
    return 0.0;
  }

  const MaterialProperty<Real> & _value;
};
