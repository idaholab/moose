//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LMKernel.h"

/**
 * Imposes a body force onto a Lagrange multiplier constrained primal equation
 */
class BodyForceLM : public LMKernel
{
public:
  BodyForceLM(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
