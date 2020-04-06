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
#include "RichardsSeffVG.h"

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
  static InputParameters validParams();

  RichardsSeff2waterVGshifted(const InputParameters & parameters);

  /**
   * water effective saturation
   * @param p porepressures.  Here (*p[0])[qp] is the water pressure at quadpoint qp, and
   * (*p[1])[qp] is the gas porepressure
   * @param qp the quadpoint to evaluate effective saturation at
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
  /// van Genuchten alpha parameter
  Real _al;

  /// van Genuchten m parameter
  Real _m;

  /// shift
  Real _shift;

  /// scale
  Real _scale;
};
