/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMVG_H
#define RICHARDSRELPERMVG_H

#include "RichardsRelPerm.h"

class RichardsRelPermVG;

template <>
InputParameters validParams<RichardsRelPermVG>();

/**
 * Van-Genuchten form of relative permeability
 * as a function of effective saturation
 */
class RichardsRelPermVG : public RichardsRelPerm
{
public:
  RichardsRelPermVG(const InputParameters & parameters);

  /**
   * relative permeability as a function of effective saturation
   * @param seff effective saturation
   */
  Real relperm(Real seff) const;

  /**
   * derivative of relative permeability wrt effective saturation
   * @param seff effective saturation
   */
  Real drelperm(Real seff) const;

  /**
   * second derivative of relative permeability wrt effective saturation
   * @param seff effective saturation
   */
  Real d2relperm(Real seff) const;

protected:
  /// immobile saturation
  Real _simm;

  /// van Genuchten m parameter
  Real _m;
};

#endif // RICHARDSRELPERMVG_H
