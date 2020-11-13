//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class CoupledArrayForce : public Kernel
{
public:
  static InputParameters validParams();

  CoupledArrayForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual RealEigenVector computeQpOffDiagJacobianArray(const ArrayMooseVariable & jvar) override;

private:
  unsigned int _v_var;
  const ArrayVariableValue & _v;
  RealEigenVector _coef;
};
