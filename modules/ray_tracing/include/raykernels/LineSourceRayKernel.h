//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericRayKernel.h"

// Forward declarations
class Function;

template <bool is_ad>
class LineSourceRayKernelTempl : public GenericRayKernel<is_ad>
{
public:
  LineSourceRayKernelTempl(const InputParameters & params);

  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;

  /// Indices into the Ray data that we want to scale the residual by (may be empty)
  const std::vector<RayDataIndex> _ray_data_factor_indices;
  /// Indices into the Ray aux data that we want to scale the residual by (may be empty)
  const std::vector<RayDataIndex> _ray_aux_data_factor_indices;

  usingGenericRayKernelMembers;
};

typedef LineSourceRayKernelTempl<false> LineSourceRayKernel;
typedef LineSourceRayKernelTempl<true> ADLineSourceRayKernel;
