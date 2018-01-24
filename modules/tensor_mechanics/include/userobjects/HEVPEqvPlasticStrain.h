/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
