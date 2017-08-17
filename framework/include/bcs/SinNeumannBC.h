/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SINNEUMANNBC_H
#define SINNEUMANNBC_H

#include "IntegratedBC.h"

// Forward Declarations
class SinNeumannBC;

template <>
InputParameters validParams<SinNeumannBC>();

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
  SinNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  Real _initial;
  Real _final;
  Real _duration;
};

#endif // SINNEUMANNBC_H
