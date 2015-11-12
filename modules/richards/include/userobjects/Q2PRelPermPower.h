/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef Q2PRELPERMPOWER_H
#define Q2PRELPERMPOWER_H

#include "RichardsRelPerm.h"

class Q2PRelPermPower;


template<>
InputParameters validParams<Q2PRelPermPower>();

/**
 * Power form of relative permeability, usually
 * used for water as a function of gas saturation
 * in the Q2P models
 */
class Q2PRelPermPower : public RichardsRelPerm
{
public:
  Q2PRelPermPower(const InputParameters & parameters);

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

#endif // Q2PRELPERMPOWER_H
