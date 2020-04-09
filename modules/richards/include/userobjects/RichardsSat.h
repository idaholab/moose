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
 * Saturation of a phase as a function of
 * effective saturation of that phase,
 * and its derivatives wrt effective saturation
 */
class RichardsSat : public GeneralUserObject
{
public:
  static InputParameters validParams();

  RichardsSat(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * saturation as a function of effective saturation
   * @param seff effective saturation
   */
  Real sat(Real seff) const;

  /// derivative of saturation wrt effective saturation
  Real dsat(Real /*seff*/) const;

protected:
  /// residual saturation for this phase
  Real _s_res;

  /// sum of the residual saturations for every phase
  Real _sum_s_res;
};
