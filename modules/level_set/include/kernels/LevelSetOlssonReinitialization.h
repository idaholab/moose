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

/**
 * Implements the re-initialization equation proposed by Olsson et. al. (2007).
 */
class LevelSetOlssonReinitialization : public ADKernelGrad
{
public:
  static InputParameters validParams();

  LevelSetOlssonReinitialization(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// Gradient of the level set variable at time, \tau = 0.
  const ADVariableGradient & _grad_levelset_0;

  /// Interface thickness
  const PostprocessorValue & _epsilon;

  /// Use modified reinitilization formulation (see Olsson et. al. (2007), section 2.2.1)
  const bool _use_modified_reinitilization_formulation;

  using ADKernelGrad::getPostprocessorValue;
};
