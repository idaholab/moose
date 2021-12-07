//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionIC.h"

/**
 * Initial conditions specifying an initial condition as a general function
 * while preserving a total integral magnitude.
 */
class IntegralPreservingFunctionIC : public FunctionIC
{
public:
  static InputParameters validParams();

  IntegralPreservingFunctionIC(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * @returns The magnitude of the function
   */
  Real magnitude() const { return _magnitude; }

protected:
  virtual Real value(const Point & p) override;

  /// Name of postprocessor providing the integral of the function
  const PostprocessorName & _pp_name;

  /// Integral of the function
  const PostprocessorValue & _integral;

  /// Magnitude of the initial condition upon integration
  const Real & _magnitude;
};
