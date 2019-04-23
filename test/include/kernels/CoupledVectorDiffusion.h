//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

class CoupledVectorDiffusion;

template <>
InputParameters validParams<CoupledVectorDiffusion>();

class CoupledVectorDiffusion : public VectorKernel
{
public:
  CoupledVectorDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  MooseEnum _state;
  const VectorVariableGradient & _grad_v;
  unsigned _v_id;
};

