//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

class Function;

/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof \vec{f} \cdot \vec{\phi_i} $
 */
class VectorBodyForce : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorBodyForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional vectorValue function
  const Function * const _function;

  /// Optional component function value
  const Function & _function_x;
  const Function & _function_y;
  const Function & _function_z;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
