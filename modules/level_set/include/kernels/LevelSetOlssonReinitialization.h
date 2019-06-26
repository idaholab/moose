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
#include "ADKernelGrad.h"

// Forward declarations
template <ComputeStage>
class LevelSetOlssonReinitialization;

declareADValidParams(LevelSetOlssonReinitialization);

/**
 * Implements the re-initialization equation proposed by Olsson et. al. (2007).
 */
template <ComputeStage compute_stage>
class LevelSetOlssonReinitialization : public ADKernelGrad<compute_stage>
{
public:
  LevelSetOlssonReinitialization(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// Gradient of the level set variable at time, \tau = 0.
  const ADVariableGradient & _grad_levelset_0;

  /// Interface thickness
  const PostprocessorValue & _epsilon;

  usingKernelGradMembers;
  using ADKernelGrad<compute_stage>::getPostprocessorValue;
};

