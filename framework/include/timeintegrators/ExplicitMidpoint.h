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

#ifndef EXPLICITMIDPOINT_H
#define EXPLICITMIDPOINT_H

#include "ExplicitRK2.h"

class ExplicitMidpoint;

template <>
InputParameters validParams<ExplicitMidpoint>();

/**
 * The explicit midpoint time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 1/2 | 1/2 0
 * ---------------------
 *     | 0   1
 *
 * See: ExplicitRK2.h for more information.
 */
class ExplicitMidpoint : public ExplicitRK2
{
public:
  ExplicitMidpoint(const InputParameters & parameters);
  virtual ~ExplicitMidpoint() {}

protected:
  /// Method coefficient overrides
  virtual Real a() const { return .5; }
  virtual Real b1() const { return 0.; }
  virtual Real b2() const { return 1.; }
};

#endif /* EXPLICITMIDPOINT_H */
