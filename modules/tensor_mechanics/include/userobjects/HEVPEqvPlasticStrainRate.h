//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HEVPInternalVarRateUOBase.h"

/**
 * This user object classs
 * Computes equivalent plastic strain rate
 */
class HEVPEqvPlasticStrainRate : public HEVPInternalVarRateUOBase
{
public:
  static InputParameters validParams();

  HEVPEqvPlasticStrainRate(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const;

protected:
  Real _h;
};
