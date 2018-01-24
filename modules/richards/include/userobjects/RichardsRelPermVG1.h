/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMVG1_H
#define RICHARDSRELPERMVG1_H

#include "RichardsRelPermVG.h"

class RichardsRelPermVG1;

template <>
InputParameters validParams<RichardsRelPermVG1>();

/**
 * Van-Genuchten form of relative permeability when seff <= _scut
 * cubic relative permeability for seff >= _scut
 * These two match in value and derivative at seff = _scut
 * and relperm = 1 for seff = 1
 */
class RichardsRelPermVG1 : public RichardsRelPermVG
{
public:
  RichardsRelPermVG1(const InputParameters & parameters);

  /// just prints some (maybe) useful info to the console
  void initialSetup();

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

  /// for seff > _scut use cubic relative permeability, otherwise use van Genuchten
  Real _scut;

  /// constant in cubic relperm relation
  Real _vg1_const;

  /// coefficient of linear term in cubic relperm relation
  Real _vg1_linear;

  /// coefficient of quadratic term in cubic relperm relation
  Real _vg1_quad;

  /// coefficient of cubic term in cubic relperm relation
  Real _vg1_cub;
};

#endif // RICHARDSRELPERMVG1_H
