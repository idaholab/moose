/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Broadbridge-White" form of relative permeability (P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water Resources Research 24 (1988) 145-154)
#ifndef RICHARDSRELPERMBW_H
#define RICHARDSRELPERMBW_H

#include "RichardsRelPerm.h"

class RichardsRelPermBW;


template<>
InputParameters validParams<RichardsRelPermBW>();

class RichardsRelPermBW : public RichardsRelPerm
{
 public:
  RichardsRelPermBW(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

 protected:

  Real _sn;
  Real _ss;
  Real _kn;
  Real _ks;
  Real _c;
  Real _coef;

};

#endif // RICHARDSRELPERMBW_H
