//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionSideIntegral.h"

/**
 * Computes the average of a function over a boundary.
 */
class FunctionSideAverage : public FunctionSideIntegral
{
public:
  static InputParameters validParams();

  FunctionSideAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  Real _volume;
};
