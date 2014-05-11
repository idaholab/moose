/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFF2WATERVG_H
#define RICHARDSSEFF2WATERVG_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2waterVG;


template<>
InputParameters validParams<RichardsSeff2waterVG>();

/**
 * van-Genuchten water effective saturation as a function of (Pwater, Pgas),
 * and its derivs wrt to those pressures.  Note that the water pressure appears
 * first in the tuple (Pwater, Pgas)
 */
class RichardsSeff2waterVG : public RichardsSeff
{
 public:
  RichardsSeff2waterVG(const std::string & name, InputParameters parameters);

  /**
   * water effective saturation
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
   */
  Real seff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * derivatives of water effective saturation wrt (Pwater, Pgas)
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
   */
  std::vector<Real> dseff(std::vector<VariableValue *> p, unsigned int qp) const;

  /**
   * second derivatives of water effective saturation wrt (Pwater, Pgas)
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

#endif // RICHARDSSEFF2WATERVG_H
