/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  van-Genuchten water effective saturation as a function of (Pwater, Pgas), and its derivs wrt to that pressure
//
#ifndef RICHARDSSEFF2WATERVG_H
#define RICHARDSSEFF2WATERVG_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2waterVG;


template<>
InputParameters validParams<RichardsSeff2waterVG>();

class RichardsSeff2waterVG : public RichardsSeff
{
 public:
  RichardsSeff2waterVG(const std::string & name, InputParameters parameters);

  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _al;
  Real _m;

};

#endif // RICHARDSSEFF2WATERVG_H
