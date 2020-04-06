//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
//  "cut" van-Genuchten effective saturation as a function of single pressure, and its derivs wrt to
//  that pressure
//

#include "RichardsSeff1VG.h"

/**
 * Effective saturation as a function of porepressure
 * using the van Genuchten formula, but when p<p_cut use a
 * linear instead, seff = a + b*p, which matches value and derivative at p=p_cut
 * This is so seff=0 at a finite value of p rather than p=-infinity.
 * Note effective saturation is not a function
 * of capillary pressure: i use porepressure instead, so
 * seff = 1 for p >= 0.
 */
class RichardsSeff1VGcut : public RichardsSeff1VG
{
public:
  static InputParameters validParams();

  RichardsSeff1VGcut(const InputParameters & parameters);

  /// just prints some (maybe) useful info to the console
  void initialSetup();

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
  /// van Genuchten alpha parameter
  Real _al;

  /// van Genuchten m parameter
  Real _m;

  /// cutoff in pressure below which use a linear relationship instead of van-Genuchten expression.  _p_cut < 0
  Real _p_cut;

  /// effective saturation at p=_p_cut
  Real _s_cut;

  /// derivative of effective saturation wrt p at p=_p_cut
  Real _ds_cut;
};
