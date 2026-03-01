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
class VariableResidualNormBase : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  VariableResidualNormBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  virtual PostprocessorValue getValue() const override;

protected:
  /// Gets the current element Dof indices
  virtual std::vector<dof_id_type> getCurrentElemDofIndices() const = 0;
  /// Computes the residual norm
  virtual void computeNorm() = 0;

  /// The variable we compute the residual for
  MooseVariableFieldBase & _var;
  /// If false, divide by residual scaling factor
  const bool _include_scaling_factor;
  /// Nonlinear residual vector
  NumericVector<Number> & _nl_residual_vector;
  /// Local DoF indices for the variable, block-restricted
  std::set<dof_id_type> _local_dof_indices;
  /// Non-local DoF indices map, indexed by the owning PID
  std::map<processor_id_type, std::vector<dof_id_type>> _nonlocal_dof_indices_map;
  /// The computed residual norm
  Real _norm;
};
