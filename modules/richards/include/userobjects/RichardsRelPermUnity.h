//  "Unity" form of relative permeability
//
#ifndef RICHARDSRELPERMUNITY_H
#define RICHARDSRELPERMUNITY_H

#include "RichardsRelPerm.h"

class RichardsRelPermUnity;


template<>
InputParameters validParams<RichardsRelPermUnity>();

class RichardsRelPermUnity : public RichardsRelPerm
{
 public:
  RichardsRelPermUnity(const std::string & name, InputParameters parameters);

  Real relperm(Real seff) const;
  Real drelperm(Real seff) const;
  Real d2relperm(Real seff) const;

};

#endif // RICHARDSRELPERMUNITY_H
