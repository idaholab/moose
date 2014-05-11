/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFF2GASVG_H
#define RICHARDSSEFF2GASVG_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2gasVG;


template<>
InputParameters validParams<RichardsSeff2gasVG>();

/**
 * van-Genuchten gas effective saturation as a function of (Pwater, Pgas),
 * and its derivs wrt to those pressures.  Note that the water pressure appears
 * first in the tuple (Pwater, Pgas)
 */
class RichardsSeff2gasVG : public RichardsSeff
{
 public:
  RichardsSeff2gasVG(const std::string & name, InputParameters parameters);

  /**
   * gas effective saturation
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
   */
  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * derivatives of gas effective saturation wrt (Pwater, Pgas)
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
   */
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * second derivatives of gas effective saturation wrt (Pwater, Pgas)
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
   */
  std::vector<std::vector<Real> > d2seff(std::vector<VariableValue *> p, unsigned int qp) const;

 protected:

  /// van Genuchten alpha parameter
  Real _al;

  /// van Genuchten m parameter
  Real _m;

};

#endif // RICHARDSSEFF2GASVG_H
