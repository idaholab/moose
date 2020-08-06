//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include <sstream>

class PFCEnergyDensity : public AuxKernel
{
public:
  static InputParameters validParams();

  PFCEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const std::vector<const VariableValue *> _vals;
  std::vector<const MaterialProperty<Real> *> _coeff;

  unsigned int _order;
  const MaterialProperty<Real> & _a;
  const MaterialProperty<Real> & _b;
};
