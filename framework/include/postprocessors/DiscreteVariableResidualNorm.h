//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * Computes a discrete norm for a block-restricted variable residual.
 */
class DiscreteVariableResidualNorm : public ElementPostprocessor
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

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  virtual PostprocessorValue getValue() const override;

protected:
  /// The variable we compute the residual for
  MooseVariableFieldBase & _var;
  /// Type of norm to compute
  const NormType _norm_type;
  /// If true, correct mesh-size bias in norm
  const bool _correct_mesh_bias;
  /// Nonlinear residual vector
  NumericVector<Number> & _nl_residual_vector;
  /// Local DoF indices for the variable, block-restricted
  std::set<dof_id_type> _local_dof_indices;
  /// Non-local DoF indices map, indexed by the owning PID
  std::map<processor_id_type, std::vector<dof_id_type>> _nonlocal_dof_indices_map;
  /// The computed residual norm
  Real _norm;
};
