//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
