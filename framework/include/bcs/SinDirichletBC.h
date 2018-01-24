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

#ifndef SINDIRICHLETBC_H
#define SINDIRICHLETBC_H

#include "NodalBC.h"

// Forward Declarations
class SinDirichletBC;

template <>
InputParameters validParams<SinDirichletBC>();

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
  SinDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  Real _initial;
  Real _final;
  Real _duration;
};

#endif
