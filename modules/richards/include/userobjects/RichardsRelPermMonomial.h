/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Monomial" form of relative permeability
//
#ifndef RICHARDSRELPERMMONOMIAL_H
#define RICHARDSRELPERMMONOMIAL_H

#include "RichardsRelPerm.h"

class RichardsRelPermMonomial;


template<>
InputParameters validParams<RichardsRelPermMonomial>();

class RichardsRelPermMonomial : public RichardsRelPerm
{
 public:
  RichardsRelPermMonomial(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

 protected:

  Real _simm;
  Real _n;

};

#endif // RICHARDSRELPERMMONOMIAL_H
