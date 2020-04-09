//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TrussMaterial.h"

class LinearElasticTruss : public TrussMaterial
{
public:
  static InputParameters validParams();

  LinearElasticTruss(const InputParameters & parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();

private:
  const VariableValue & _T;

  Real _T0;
  Real _thermal_expansion_coeff;
};
