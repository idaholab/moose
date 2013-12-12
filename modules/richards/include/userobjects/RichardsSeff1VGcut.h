//  "cut" van-Genuchten effective saturation as a function of capillary pressure, and its derivs wrt pc
//
#ifndef RICHARDSSEFFVG1_H
#define RICHARDSSEFFVG1_H

#include "RichardsSeffVG.h"

class RichardsSeffVG1;


template<>
InputParameters validParams<RichardsSeffVG1>();

class RichardsSeffVG1 : public RichardsSeffVG
{
 public:
  RichardsSeffVG1(const std::string & name, InputParameters parameters);

  Real seff(Real pc) const;
  Real dseff(Real pc) const;
  Real d2seff(Real pc) const;

 protected:
  
  Real _al;
  Real _m;
  Real _p_cut;
  Real _s_cut;
  Real _ds_cut;
    

};

#endif // RICHARDSSEFFVG1_H
