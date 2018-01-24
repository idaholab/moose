//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFC_ENERGY_DENSITY_H
#define PFC_ENERGY_DENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCEnergyDensity;

template <>
InputParameters validParams<PFCEnergyDensity>();

class PFCEnergyDensity : public AuxKernel
{
public:
  PFCEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  std::vector<const VariableValue *> _vals;
  std::vector<const MaterialProperty<Real> *> _coeff;

  unsigned int _order;
  const MaterialProperty<Real> & _a;
  const MaterialProperty<Real> & _b;
};

#endif // PFC_ENERGY_DENSITY_H
