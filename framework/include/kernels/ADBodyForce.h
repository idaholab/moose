//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

// Forward Declaration
template <ComputeStage>
class ADBodyForce;
class Function;

declareADValidParams(ADBodyForce);

/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof f \cdot \phi_i $
 *
 * The coefficient and function both have defaults
 * equal to 1.0.
 */
template <ComputeStage compute_stage>
class ADBodyForce : public ADKernelValue<compute_stage>
{
public:
  ADBodyForce(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;

  usingKernelValueMembers;
  using KernelBase::_q_point;
};
