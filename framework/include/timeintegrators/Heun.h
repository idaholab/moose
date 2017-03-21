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

#ifndef HEUN_H
#define HEUN_H

#include "ExplicitRK2.h"

class Heun;

template <>
InputParameters validParams<Heun>();

/**
 * Heun's (aka improved Euler) time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 1   | 1    0
 * ---------------------
 *     | 1/2  1/2
 *
 * See: ExplicitRK2.h for more information.
 */
class Heun : public ExplicitRK2
{
public:
  Heun(const InputParameters & parameters);
  virtual ~Heun() {}

protected:
  /// Method coefficient overrides
  virtual Real a() const { return 1.; }
  virtual Real b1() const { return .5; }
  virtual Real b2() const { return .5; }
};

#endif /* HEUN_H */
