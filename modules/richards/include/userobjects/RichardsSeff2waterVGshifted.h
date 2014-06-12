/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFF2WATERVGSHIFTED_H
#define RICHARDSSEFF2WATERVGSHIFTED_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2waterVGshifted;


template<>
InputParameters validParams<RichardsSeff2waterVGshifted>();

/**
 * Shifted van-Genuchten water effective saturation as a function of (Pwater, Pgas),
 * and its derivs wrt to those pressures.  Note that the water pressure appears
 * first in the tuple (Pwater, Pgas)
 * This takes the original van-Genuchten Seff = Seff(Pwater-Pgas), and shifts it
 * to the right by "shift", and scales the result so 0<=Seff<=1.
 * The purpose of this is so dSeff/dP>0 at P=0.
 */
class RichardsSeff2waterVGshifted : public RichardsSeff
{
 public:
  RichardsSeff2waterVGshifted(const std::string & name, InputParameters parameters);

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

  /// shift
  Real _shift;

  /// scale
  Real _scale;

};

#endif // RICHARDSSEFF2WATERVGSHIFTED_H
