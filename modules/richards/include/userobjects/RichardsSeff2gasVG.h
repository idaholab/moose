/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  van-Genuchten gaseffective saturation as a function of (Pwater, Pgas), and its derivs wrt to that pressure
//
#ifndef RICHARDSSEFF2GASVG_H
#define RICHARDSSEFF2GASVG_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2gasVG;


template<>
InputParameters validParams<RichardsSeff2gasVG>();

class RichardsSeff2gasVG : public RichardsSeff
{
 public:
  RichardsSeff2gasVG(const std::string & name, InputParameters parameters);

  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _al;
  Real _m;

};

#endif // RICHARDSSEFF2GASVG_H
