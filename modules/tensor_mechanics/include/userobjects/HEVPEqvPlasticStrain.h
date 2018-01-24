//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HEVPEQVPLASTICSTRAIN_H
#define HEVPEQVPLASTICSTRAIN_H

#include "HEVPInternalVarUOBase.h"

class HEVPEqvPlasticStrain;

template <>
InputParameters validParams<HEVPEqvPlasticStrain>();

/**
 * This user object classs
 * Computes equivalent plastic strain
 */
class HEVPEqvPlasticStrain : public HEVPInternalVarUOBase
{
public:
  HEVPEqvPlasticStrain(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real, Real &) const;
  virtual bool computeDerivative(unsigned int, Real, const std::string &, Real &) const;
};

#endif
