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

#ifndef RALSTON_H
#define RALSTON_H

#include "ExplicitRK2.h"

class Ralston;

template <>
InputParameters validParams<Ralston>();

/**
 * Ralston's time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 2/3 | 2/3    0
 * ---------------------
 *     | 1/4  3/4
 *
 * See: ExplicitRK2.h for more information.
 */
class Ralston : public ExplicitRK2
{
public:
  Ralston(const InputParameters & parameters);
  virtual ~Ralston() {}

protected:
  /// Method coefficient overrides
  virtual Real a() const { return 2. / 3.; }
  virtual Real b1() const { return .25; }
  virtual Real b2() const { return .75; }
};

#endif /* RALSTON_H */
