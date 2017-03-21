/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPLINEARHARDENING_H
#define HEVPLINEARHARDENING_H

#include "HEVPStrengthUOBase.h"

class HEVPLinearHardening;

template <>
InputParameters validParams<HEVPLinearHardening>();

/**
 * This user object classs
 * Computes linear hardening
 */
class HEVPLinearHardening : public HEVPStrengthUOBase
{
public:
  HEVPLinearHardening(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const;

protected:
  Real _sig0;
  Real _slope;
};

#endif
