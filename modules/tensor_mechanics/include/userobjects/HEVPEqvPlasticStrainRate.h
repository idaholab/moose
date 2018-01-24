//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HEVPEQVPLASTICSTRAINRATE_H
#define HEVPEQVPLASTICSTRAINRATE_H

#include "HEVPInternalVarRateUOBase.h"

class HEVPEqvPlasticStrainRate;

template <>
InputParameters validParams<HEVPEqvPlasticStrainRate>();

/**
 * This user object classs
 * Computes equivalent plastic strain rate
 */
class HEVPEqvPlasticStrainRate : public HEVPInternalVarRateUOBase
{
public:
  HEVPEqvPlasticStrainRate(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const;

protected:
  Real _h;
};

#endif
