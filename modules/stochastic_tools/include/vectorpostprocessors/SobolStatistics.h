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
#include "SamplerInterface.h"

class SobolSampler;

/**
 * Computes Sobol sensitivity indices, see SobolCalculators
 */
class SobolStatistics : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  static InputParameters validParams();
  SobolStatistics(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialSetup() override;

  /// Not used; all parallel computation is wrapped in the SobolCalculator objects
  virtual void initialize() final{};
  virtual void finalize() final{};

protected:
  /// The sampler that generated the samples that produced results for the _results_vectors
  const SobolSampler & _sobol_sampler;

  /// Result vectors from StocasticResults object
  std::vector<std::pair<const VectorPostprocessorValue *, bool>> _result_vectors;

  /// Vectors computed by this object
  std::vector<VectorPostprocessorValue *> _sobol_stat_vectors;

  /// PrefGraph timer
  const PerfID _perf_execute;
};
