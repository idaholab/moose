/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFF2GASVGSHIFTED_H
#define RICHARDSSEFF2GASVGSHIFTED_H

#include "RichardsSeff.h"
#include "RichardsSeffVG.h"

class RichardsSeff2gasVGshifted;


template<>
InputParameters validParams<RichardsSeff2gasVGshifted>();

/**
 * Shifted van-Genuchten water effective saturation as a function of (Pwater, Pgas),
 * and its derivs wrt to those pressures.  Note that the water pressure appears
 * first in the tuple (Pwater, Pgas)
 * This takes the original van-Genuchten Seff = Seff(Pwater-Pgas), and shifts it
 * to the right by "shift", and scales the result so 0<=Seff<=1.
 * The purpose of this is so dSeff/dP>0 at P=0.
 */
class RichardsSeff2gasVGshifted : public RichardsSeff
{
 public:
  RichardsSeff2gasVGshifted(const std::string & name, InputParameters parameters);

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

  /// shift
  Real _shift;

  /// scale
  Real _scale;

};

#endif // RICHARDSSEFF2GASVGSHIFTED_H
