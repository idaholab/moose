//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFCRFF_ENERGY_DENSITY_H
#define PFCRFF_ENERGY_DENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCRFFEnergyDensity;

template <>
InputParameters validParams<PFCRFFEnergyDensity>();

class PFCRFFEnergyDensity : public AuxKernel
{
public:
  PFCRFFEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  unsigned int _order;
  std::vector<const VariableValue *> _vals;

  Real _a;
  Real _b;
  Real _c;
  unsigned int _num_exp_terms;
  MooseEnum _log_approach;
  Real _tol;
};

#endif // PFCRFF_ENERGY_DENSITY_H
