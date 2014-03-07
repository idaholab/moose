/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Broadbridge-White" form of effective saturation for Kn small (P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water Resources Research 24 (1988) 145-154)
//
#ifndef RICHARDSSEFF1BWSMALL_H
#define RICHARDSSEFF1BWSMALL_H

#include "RichardsSeff.h"
//#include "LambertW.h"

class RichardsSeff1BWsmall;


template<>
InputParameters validParams<RichardsSeff1BWsmall>();

class RichardsSeff1BWsmall : public RichardsSeff
{
 public:
  RichardsSeff1BWsmall(const std::string & name, InputParameters parameters);

  Real LambertW(const double z) const;
  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _sn;
  Real _ss;
  Real _c;
  Real _las;

};

#endif // RICHARDSSEFF1BWSMALL_H
