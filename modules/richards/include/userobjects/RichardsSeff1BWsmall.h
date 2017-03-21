/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSEFF1BWSMALL_H
#define RICHARDSSEFF1BWSMALL_H

#include "RichardsSeff.h"

class RichardsSeff1BWsmall;

template <>
InputParameters validParams<RichardsSeff1BWsmall>();

/**
 * "Broadbridge-White" form of effective saturation for Kn small
 * as a function of porepressure (not capillary pressure, so
 * Seff = 1 for p>=0).  See
 * P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1.
 * Analytic Solution'', Water Resources Research 24 (1988) 145-154
 */
class RichardsSeff1BWsmall : public RichardsSeff
{
public:
  RichardsSeff1BWsmall(const InputParameters & parameters);

  /**
   * LambertW function, returned value satisfies W(z)*exp(W(z))=z
   * @param z the z value in above expression
   */
  Real LambertW(const double z) const;

  /**
   * effective saturation as a function of porepressure
   * @param p porepressure in the element.  Note that (*p[0])[qp] is the porepressure at quadpoint
   * qp
   * @param qp the quad point to evaluate effective saturation at
   */
  Real seff(std::vector<const VariableValue *> p, unsigned int qp) const;

  /**
   * derivative of effective saturation as a function of porepressure
   * @param p porepressure in the element.  Note that (*p[0])[qp] is the porepressure at quadpoint
   * qp
   * @param qp the quad point to evaluate effective saturation at
   * @param result the derivtives will be placed in this array
   */
  void
  dseff(std::vector<const VariableValue *> p, unsigned int qp, std::vector<Real> & result) const;

  /**
   * second derivative of effective saturation as a function of porepressure
   * @param p porepressure in the element.  Note that (*p[0])[qp] is the porepressure at quadpoint
   * qp
   * @param qp the quad point to evaluate effective saturation at
   * @param result the derivtives will be placed in this array
   */
  void d2seff(std::vector<const VariableValue *> p,
              unsigned int qp,
              std::vector<std::vector<Real>> & result) const;

protected:
  /// BW's initial effective saturation
  Real _sn;

  /// Effective saturation where porepressure = 0
  Real _ss;

  /// BW's C parameter
  Real _c;

  /// BW's lambda_s parameter multiplied by (fluiddensity*gravity)
  Real _las;
};

#endif // RICHARDSSEFF1BWSMALL_H
