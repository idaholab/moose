/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMPOWER_H
#define RICHARDSRELPERMPOWER_H

#include "RichardsRelPerm.h"

class RichardsRelPermPower;

template <>
InputParameters validParams<RichardsRelPermPower>();

/**
 * Power form of relative permeability, usually
 * used for water
 */
class RichardsRelPermPower : public RichardsRelPerm
{
public:
  RichardsRelPermPower(const InputParameters & parameters);

  /**
   * relative permeability as a function of effective saturation
   * @param seff effective sasturation
   */
  Real relperm(Real seff) const;

  /**
   * derivative of relative permeability wrt effective saturation
   * @param seff effective sasturation
   */
  Real drelperm(Real seff) const;

  /**
   * second derivative of relative permeability wrt effective saturation
   * @param seff effective sasturation
   */
  Real d2relperm(Real seff) const;

protected:
  /// immobile saturation
  Real _simm;

  /// exponent used in the power relationship
  Real _n;
};

#endif // RICHARDSRELPERMPOWER_H
