//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RichardsSeff.h"

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
  static InputParameters validParams();

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
