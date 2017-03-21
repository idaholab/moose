/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
