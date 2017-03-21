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
#ifndef STEADYWITHNULL_H
#define STEADYWITHNULL_H

#include "Steady.h"

class SteadyWithNull;

template <>
InputParameters validParams<SteadyWithNull>();

/**
 * Steady excecutioner setting nullspace
 */
class SteadyWithNull : public Steady
{
public:
  SteadyWithNull(const InputParameters & parameters);

  virtual void init() override;
};

#endif /* SteadyWithNull_H */
