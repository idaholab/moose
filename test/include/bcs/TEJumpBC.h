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
class Function;

/**
 * Implements a BC for TimeError test case
 *
 */
class TEJumpBC : public NodalBC
{
public:
  static InputParameters validParams();

  TEJumpBC(const InputParameters & parameters);

  virtual ~TEJumpBC() {}

protected:
  virtual Real computeQpResidual();

  Real _t_jump;
  Real _slope;
};
