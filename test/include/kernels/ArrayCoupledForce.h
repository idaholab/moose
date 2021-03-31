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

class ArrayCoupledForce : public ArrayKernel
{
public:
  static InputParameters validParams();

  ArrayCoupledForce(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;

  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

private:
  const bool _is_v_array;
  const unsigned int _v_var;
  const VariableValue * const _v;
  const ArrayVariableValue * const _v_array;
  const RealEigenVector _coef;
};
