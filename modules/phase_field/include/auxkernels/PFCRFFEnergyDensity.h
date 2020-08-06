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

class PFCRFFEnergyDensity : public AuxKernel
{
public:
  static InputParameters validParams();

  PFCRFFEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const unsigned int _order;
  const std::vector<const VariableValue *> _vals;

  const Real _a;
  const Real _b;
  const Real _c;
  const unsigned int _num_exp_terms;
  const MooseEnum _log_approach;
  const Real _tol;
};
