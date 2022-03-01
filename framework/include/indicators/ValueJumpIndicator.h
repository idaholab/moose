//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideIndicator.h"

class ValueJumpIndicator : public InternalSideIndicator
{
public:
  static InputParameters validParams();

  ValueJumpIndicator(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
