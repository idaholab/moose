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
#include "FullSolveMultiApp.h"
#include "SamplerInterface.h"

#include "StochasticToolsTypes.h"

class Sampler;
class StochasticToolsTransfer;

class SamplerFullSolveMultiApp : public FullSolveMultiApp, public SamplerInterface
{
public:
  static InputParameters validParams();
  SamplerFullSolveMultiApp(const InputParameters & parameters);
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;
  virtual void preTransfer(Real dt, Real target_time) override;

protected:
  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;

  /// The Sup-application solve mode
  const StochasticTools::MultiAppMode _mode;

  /// Counter for extracting command line arguments in batch mode
  dof_id_type _local_batch_app_index;

  /// Override to allow for batch mode to get correct cli_args
  virtual std::string getCommandLineArgsParamHelper(unsigned int local_app) override;

private:
  /**
   * Helper method for running in mode='batch'
   * */
  bool solveStepBatch(Real dt, Real target_time, bool auto_advance = true);

  /**
   * Checks whether the MultiApp partitioning is consistent with the sampler
   */
  void checkRankConfig();

  /**
   * Helper function for updating _row_data and _local_row_index.
   * This allows multiple calls to the same row index
   */
  void updateRowData(dof_id_type local_index);

  /**
   * Helper for getting StochasticToolsTransfer objects.
   */
  std::vector<std::shared_ptr<StochasticToolsTransfer>>
  getActiveStochasticToolsTransfers(Transfer::DIRECTION direction);

  // Flag indicating a solve has occured
  bool _solved_once;

  ///@{
  /// PrefGraph timers
  const PerfID _perf_solve_step;
  const PerfID _perf_solve_batch_step;
  const PerfID _perf_command_line_args;
  ///@}

  // Sampler size, to test if the MultiApp object needs to be re-initialize
  dof_id_type _number_of_sampler_rows;

  /// Current row of data updated by updateRowData. Used by transfers and setting command line args
  std::vector<Real> _row_data;
  /// Current local index representing _row_data
  dof_id_type _local_row_index = std::numeric_limits<dof_id_type>::max();
};
