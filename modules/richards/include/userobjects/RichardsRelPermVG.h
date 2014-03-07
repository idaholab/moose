/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  van-Genuchten form of relative permeability
//
#ifndef RICHARDSRELPERMVG_H
#define RICHARDSRELPERMVG_H

#include "RichardsRelPerm.h"

class RichardsRelPermVG;


template<>
InputParameters validParams<RichardsRelPermVG>();

class RichardsRelPermVG : public RichardsRelPerm
{
 public:
  RichardsRelPermVG(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

 protected:

  Real _simm;
  Real _m;

};

#endif // RICHARDSRELPERMVG_H
