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
#include "StochasticToolsTransfer.h"
#include "Sampler.h"

// Forward declarations
class SamplerPostprocessorTransfer;
class SamplerReceiver;
class SamplerFullSolveMultiApp;
class StochasticResults;

template <>
InputParameters validParams<SamplerPostprocessorTransfer>();

/**
 * Transfer Postprocessor from sub-applications to the master application.
 */
class SamplerPostprocessorTransfer : public StochasticToolsTransfer
{
public:
  SamplerPostprocessorTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  /**
   * Traditional Transfer callback
   */
  virtual void execute() override;

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeFromMultiapp() override;
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override;
  ///@}

  /// Sampler object that is retrieved from the SamplerTransientMultiApp or SamplerFullSolveMultiApp
  Sampler * _sampler;

  /// Storage for StochasticResults object that data will be transferred to/from
  StochasticResults * _results;

  /// Local values of compute PP values
  std::vector<PostprocessorValue> _local_values;

  /// Name of postprocessor on the sub-applications
  const PostprocessorName & _sub_pp_name;

  /// Name of vector-postprocessor on the master
  const VectorPostprocessorName & _master_vpp_name;
};
