/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "cut" van-Genuchten effective saturation as a function of single pressure, and its derivs wrt to that pressure
//
#ifndef RICHARDSSEFF1VGCUT_H
#define RICHARDSSEFF1VGCUT_H

#include "RichardsSeff1VG.h"

class RichardsSeff1VGcut;


template<>
InputParameters validParams<RichardsSeff1VGcut>();

class RichardsSeff1VGcut : public RichardsSeff1VG
{
 public:
  RichardsSeff1VGcut(const std::string & name, InputParameters parameters);

  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _al;
  Real _m;
  Real _p_cut;
  Real _s_cut;
  Real _ds_cut;


};

#endif // RICHARDSSEFF1VGCUT_H
