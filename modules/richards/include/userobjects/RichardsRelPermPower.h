/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Power" form of relative permeability
//
#ifndef RICHARDSRELPERMPOWER_H
#define RICHARDSRELPERMPOWER_H

#include "RichardsRelPerm.h"

class RichardsRelPermPower;


template<>
InputParameters validParams<RichardsRelPermPower>();

class RichardsRelPermPower : public RichardsRelPerm
{
 public:
  RichardsRelPermPower(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

 protected:

  Real _simm;
  Real _n;

};

#endif // RICHARDSRELPERMPOWER_H
