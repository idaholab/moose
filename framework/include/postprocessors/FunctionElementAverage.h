//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionElementIntegral.h"

/**
 * Computes the average of a function over a volume.
 */
class FunctionElementAverage : public FunctionElementIntegral
{
public:
  static InputParameters validParams();

  FunctionElementAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  const Real & getVolume() const { return _volume; }
  const Real & getIntegralValue() const { return _integral_value; }

protected:
  Real _volume;
};
