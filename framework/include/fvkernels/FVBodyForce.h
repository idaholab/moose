//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof f$
 *
 * The coefficient and function both have defaults
 * equal to 1.0.
 */
class FVBodyForce : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVBodyForce(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
