//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredAverage.h"
#include "RZSymmetry.h"

/**
 * The same functionality as LayeredAverage but for arbitrary RZ symmetry
 *
 * NOTE: this is a temporary object until MOOSE can do this
 */
class LayeredAverageRZ : public LayeredAverage, public RZSymmetry
{
public:
  LayeredAverageRZ(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual Real computeIntegral() override;

public:
  static InputParameters validParams();
};
