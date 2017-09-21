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

#ifndef USERFORCINGFUNCTION_H
#define USERFORCINGFUNCTION_H

#include "BodyForce.h"

/**
 * Deprecated, use BodyForce.
 */
class UserForcingFunction : public BodyForce
{
public:
  UserForcingFunction(const InputParameters & parameters);

protected:
  Real f();
};

template <>
InputParameters validParams<UserForcingFunction>();

#endif
