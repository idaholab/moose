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

class DGCoupledDiffusion : public DGKernel
{
public:
  static InputParameters validParams();

  DGCoupledDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  MooseVariable & _v_var;
  const VariableValue & _v;
  const VariableValue & _v_neighbor;
  const VariableGradient & _grad_v;
  const VariableGradient & _grad_v_neighbor;
  unsigned int _v_id;
};
