/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Power" form of relative permeability
//
#ifndef RICHARDSRELPERMVG1_H
#define RICHARDSRELPERMVG1_H

#include "RichardsRelPermVG.h"

class RichardsRelPermVG1;


template<>
InputParameters validParams<RichardsRelPermVG1>();

class RichardsRelPermVG1 : public RichardsRelPermVG
{
 public:
  RichardsRelPermVG1(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

 protected:

  Real _simm;
  Real _m;
  Real _scut;
  Real _vg1_const;
  Real _vg1_linear;
  Real _vg1_quad;
  Real _vg1_cub;

};

#endif // RICHARDSRELPERMVG1_H
