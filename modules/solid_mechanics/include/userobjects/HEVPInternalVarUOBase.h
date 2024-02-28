//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiscreteElementUserObject.h"
#include "RankTwoTensor.h"

/**
 * This user object is a pure virtual base classs
 * Derived classes integrate internal variables
 * Currently only old state is retrieved to use backward Euler
 */
class HEVPInternalVarUOBase : public DiscreteElementUserObject
{
public:
  static InputParameters validParams();

  HEVPInternalVarUOBase(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real, Real &) const = 0;
  virtual bool computeDerivative(unsigned int, Real, const std::string &, Real &) const = 0;

protected:
  std::string _intvar_rate_prop_name;
  const MaterialProperty<Real> & _intvar_rate;
  const MaterialProperty<Real> & _this_old;
};
