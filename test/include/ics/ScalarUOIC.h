//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarInitialCondition.h"

class ScalarUOIC;
class MTUserObject;

template <>
InputParameters validParams<ScalarUOIC>();

/**
 * Scalar initial condition for setting values from a user object
 */
class ScalarUOIC : public ScalarInitialCondition
{
public:
  ScalarUOIC(const InputParameters & parameters);

  virtual Real value() override;

protected:
  const MTUserObject & _uo;
};
