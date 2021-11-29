//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

/**
 * A spatially-constant, time-varying NodalBC whose imposed value g(t)
 * varies sinusoidally with time according to the formula:
 *
 * g(t) = { g0 + (gT - g0) * sin ((pi*t) / (2*T)), 0 < t < T
 *        { gT                                   , t > T
 * where:
 * g0 = value at time 0
 * gT = value at time T
 *  T = duration over which the value is changing.
 */
class SinDirichletBC : public NodalBC
{
public:
  static InputParameters validParams();

  SinDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  Real _initial;
  Real _final;
  Real _duration;
};
