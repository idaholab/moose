//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

// Forward Declarations
class FunctionSideIntegral;
class Function;

template <>
InputParameters validParams<FunctionSideIntegral>();

/**
 * This postprocessor computes the integral of a function over a specified boundary
 */
class FunctionSideIntegral : public SideIntegralPostprocessor
{
public:
  FunctionSideIntegral(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;

  /// The function
  const Function & _func;
};
