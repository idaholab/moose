//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "Calculators.h"
#include "BootstrapCalculators.h"

class SobolStatistics;
class SobolSampler;

template <>
InputParameters validParams<SobolStatistics>();

class SobolStatistics : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  static InputParameters validParams();
  SobolStatistics(const InputParameters & parameters);
  virtual void execute() override;

  /// Not used; all parallel computation is wrapped in the SobolCalculator objects
  virtual void initialize() final{};
  virtual void finalize() final{};

protected:
  const SobolSampler & _sobol_sampler;

  dof_id_type _num_rows_per_matrix;
  dof_id_type _num_cols_per_matrix;

  const bool _is_result_distributed = false;

  VectorPostprocessorValue & _stat_ids;

  std::vector<std::pair<const VectorPostprocessorValue *, bool>> _result_vectors;
  std::vector<VectorPostprocessorValue *> _sobol_stat_vectors;
};
