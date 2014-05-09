/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSRELPERMVG_H
#define RICHARDSRELPERMVG_H

#include "RichardsRelPerm.h"

class RichardsRelPermVG;


template<>
InputParameters validParams<RichardsRelPermVG>();

/**
 * Van-Genuchten form of relative permeability
 * as a function of effective saturation
 */
class RichardsRelPermVG : public RichardsRelPerm
{
 public:
  RichardsRelPermVG(const std::string & name, InputParameters parameters);

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
