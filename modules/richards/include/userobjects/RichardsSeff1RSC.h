/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Rogers-Stallybrass-Clements version of effective saturation of water phase.  This is mostly useful for 2phase, not single phase, models.
//  valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important here!).
// C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and Applications 7 (1983) 785--799.
//
#ifndef RICHARDSSEFF1RSC_H
#define RICHARDSSEFF1RSC_H

#include "RichardsSeff.h"
#include "RichardsSeffRSC.h"

class RichardsSeff1RSC;


template<>
InputParameters validParams<RichardsSeff1RSC>();

class RichardsSeff1RSC : public RichardsSeff
{
 public:
  RichardsSeff1RSC(const std::string & name, InputParameters parameters);

  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  Real _oil_viscosity;
  Real _scale_ratio;
  Real _shift;
  Real _scale;

};

#endif // RICHARDSSEFF1RSC_H
