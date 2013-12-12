//  van-Genuchten effective saturation as a function of capillary pressure, and its derivs wrt pc
//
#ifndef RICHARDSSEFFVG_H
#define RICHARDSSEFFVG_H

#include "RichardsSeff.h"

class RichardsSeffVG;


template<>
InputParameters validParams<RichardsSeffVG>();

class RichardsSeffVG : public RichardsSeff
{
 public:
  RichardsSeffVG(const std::string & name, InputParameters parameters);

  Real seff(Real pc) const;
  Real dseff(Real pc) const;
  Real d2seff(Real pc) const;

 protected:
  
  Real _al;
  Real _m;

};

#endif // RICHARDSSEFFVG_H
