/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  van-Genuchten effective saturation as a function of single pressure, and its derivs wrt to that pressure
//
#ifndef RICHARDSSEFF1VG_H
#define RICHARDSSEFF1VG_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff1VG;


template<>
InputParameters validParams<RichardsSeff1VG>();

class RichardsSeff1VG : public RichardsSeff
{
 public:
  RichardsSeff1VG(const std::string & name, InputParameters parameters);

  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _al;
  Real _m;

};

#endif // RICHARDSSEFF1VG_H
