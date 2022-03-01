//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Implements a spatially-constant, time-varying flux boundary
 * condition grad(u).n = g(t), where
 *
 * g(t) = { g0 + (gT - g0) * sin ((pi*t) / (2*T)), 0 < t < T
 *        { gT                                   , t > T
 *
 * and where:
 * g0 = value at time 0
 * gT = value at time T
 *  T = duration over which the value is changing.
 */
class SinNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  SinNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  Real _initial;
  Real _final;
  Real _duration;
};
