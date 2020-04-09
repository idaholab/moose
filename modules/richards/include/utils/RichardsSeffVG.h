//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Utility functions for van-genuchten effective saturation
 * as a function of porepressure (not capillary pressure),
 * and first and second derivs wrt porepressure.
 * So seff = 1 for p >= 0.
 *    seff < 1 for p < 0.
 */
class RichardsSeffVG
{
public:
  /**
   * effective saturation as a fcn of porepressure
   * @param p porepressure
   * @param al van-genuchten alpha parameter
   * @param m van-genuchten m parameter
   */
  static Real seff(Real p, Real al, Real m);

  /**
   * derivative of effective saturation wrt porepressure
   * @param p porepressure
   * @param al van-genuchten alpha parameter
   * @param m van-genuchten m parameter
   */
  static Real dseff(Real p, Real al, Real m);

  /**
   * 2nd derivative of effective saturation wrt porepressure
   * @param p porepressure
   * @param al van-genuchten alpha parameter
   * @param m van-genuchten m parameter
   */
  static Real d2seff(Real p, Real al, Real m);
};
