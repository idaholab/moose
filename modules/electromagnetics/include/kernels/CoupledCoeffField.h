//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCoupledForce.h"

class CoupledCoeffField : public ADCoupledForce
{
public:
  static InputParameters validParams();

  CoupledCoeffField(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const Function & _func;

  Real _sign;
};
