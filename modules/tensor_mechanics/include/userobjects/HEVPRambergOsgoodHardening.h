/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPRAMBERGOSGOODHARDENING_H
#define HEVPRAMBERGOSGOODHARDENING_H

#include "HEVPStrengthUOBase.h"

class HEVPRambergOsgoodHardening;

template <>
InputParameters validParams<HEVPRambergOsgoodHardening>();

/**
 * This user object classs
 * Computes power law  hardening
 */
class HEVPRambergOsgoodHardening : public HEVPStrengthUOBase
{
public:
  HEVPRambergOsgoodHardening(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const;

protected:
  Real _sig0;
  Real _peeq0;
  Real _exponent;
};

#endif
