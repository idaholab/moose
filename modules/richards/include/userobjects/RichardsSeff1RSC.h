/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFF1RSC_H
#define RICHARDSSEFF1RSC_H

#include "RichardsSeff.h"
#include "RichardsSeffRSC.h"

class RichardsSeff1RSC;


template<>
InputParameters validParams<RichardsSeff1RSC>();

/**
 * Rogers-Stallybrass-Clements version of effective saturation for single-phase simulations
 * as a function of porepressure, and its derivs wrt to that pressure.
 * Note that this is mostly useful for 2phase simulations, not this singlephase version.
 * Valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important here!).
 * C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and Applications 7 (1983) 785--799.
 */
class RichardsSeff1RSC : public RichardsSeff
{
 public:
  RichardsSeff1RSC(const std::string & name, InputParameters parameters);

  /**
   * water effective saturation
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp
   * @param qp the quadpoint to evaluate effective saturation at
   */
  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * derivative of water effective saturation wrt porepressure
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp
   * @param qp the quadpoint to evaluate effective saturation at
   */
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * second derivative of water effective saturation wrt porepressure
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp
   * @param qp the quadpoint to evaluate effective saturation at
   */
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  /// oil viscosity
  Real _oil_viscosity;

  /// RSC scale ratio
  Real _scale_ratio;

  /// RSC shift
  Real _shift;

  /// RSC scale
  Real _scale;

};

#endif // RICHARDSSEFF1RSC_H
