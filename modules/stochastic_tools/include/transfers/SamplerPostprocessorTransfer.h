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

// Forward declarations
class SamplerPostprocessorTransfer;
class SamplerReceiver;
class SamplerFullSolveMultiApp;
class StochasticResults;

template <>
InputParameters validParams<SamplerPostprocessorTransfer>();

/**
 * Transfer Postprocessor from sub-applications to a VectorPostprocessor on the master application.
 *
 * This object transfers the distributed data to a StochasticResults object.
 */
class SamplerPostprocessorTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();

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

  /// Storage for StochasticResults object that data will be transferred to/from
  StochasticResults * _results;

  /// Name of postprocessor on the sub-applications
  const PostprocessorName & _sub_pp_name;

  /// Name of vector-postprocessor on the master
  const VectorPostprocessorName & _master_vpp_name;

  /// Temporary storage for batch mode execution
  VectorPostprocessorValue _current_data;
};
