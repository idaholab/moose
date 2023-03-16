//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideUserObject.h"

/**
 * This postprocessor computes a side integral of the specified variable over a given boundary.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  SideIntegralUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override {}

  /// Returns the integral value
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};
