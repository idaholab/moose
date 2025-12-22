//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VariableResidualNormBase.h"

/**
 * Computes a discrete norm for a block-restricted variable residual.
 */
class DiscreteVariableResidualNorm : public VariableResidualNormBase
{
public:
  static InputParameters validParams();

  /// Type of discrete norm
  enum class NormType
  {
    l_1 = 0,
    l_2 = 1,
    l_inf = 2
  };

  DiscreteVariableResidualNorm(const InputParameters & parameters);

protected:
  virtual std::vector<dof_id_type> getCurrentElemDofIndices() const override;
  virtual void computeNorm() override;

  /// Type of norm to compute
  const NormType _norm_type;
  /// If true, correct mesh-size bias in norm
  const bool _correct_mesh_bias;
};
