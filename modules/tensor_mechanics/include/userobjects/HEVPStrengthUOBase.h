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
 * Derived classes computes material resistances and derivatives
 */
class HEVPStrengthUOBase : public DiscreteElementUserObject
{
public:
  static InputParameters validParams();

  HEVPStrengthUOBase(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const = 0;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const = 0;

protected:
  std::string _intvar_prop_name;
  const MaterialProperty<Real> & _intvar;
};
