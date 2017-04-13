/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMBW_H
#define RICHARDSRELPERMBW_H

#include "RichardsRelPerm.h"

class RichardsRelPermBW;

template <>
InputParameters validParams<RichardsRelPermBW>();

/**
 * "Broadbridge-White" form of relative permeability as a function of effective saturation
 * P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1.
 * Analytic Solution'', Water Resources Research 24 (1988) 145-154)
 */
class RichardsRelPermBW : public RichardsRelPerm
{
public:
  RichardsRelPermBW(const InputParameters & parameters);

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
  // BW's initial saturation
  Real _sn;

  // BW's effective saturation where porepressure = 0
  Real _ss;

  // relative perm at Seff = Sn
  Real _kn;

  // relative perm at Seff = Ss
  Real _ks;

  // BW's C parameter
  Real _c;

  // (_ks - _kn)*(_c - 1)
  Real _coef;
};

#endif // RICHARDSRELPERMBW_H
