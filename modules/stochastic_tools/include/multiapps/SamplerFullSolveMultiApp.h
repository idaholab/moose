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

class SamplerFullSolveMultiApp : public FullSolveMultiApp,
                                 public SamplerInterface,
                                 public ReporterInterface
{
public:
  static InputParameters validParams();
  SamplerFullSolveMultiApp(const InputParameters & parameters);
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;
  virtual void preTransfer(Real dt, Real target_time) override;

  /**
   * Helper for inserting row data into commandline arguments
   * Used here and in SamplerTransientMultiApp
   *
   * How it works:
   * - Scalar parameters are done in order of row data:
   *      param1;param2;param3 -> param1=row[0] param2=row[1] param3=row[2]
   * - Vector parameters are assigned with brackets:
   *      vec_param1[0,1];vec_param2[1,2] -> vec_param1='row[0] row[1]' vec_param2='row[1] row[2]'
   * - Any parameter already with an equal sign is not modified:
   *      param1=3.14;param2[0,1,2] -> param1=3.14 param2='row[0] row[1] row[2]'
   */
  static std::string sampledCommandLineArgs(const std::vector<Real> & row,
                                            const std::vector<std::string> & full_args_name);

  /**
   * Helper for executing transfers when doing batch stochastic simulations
   *
   * @param transfers A vector of transfers to execute
   * @param global_row_index The global row index of the run
   * @param row_data The current sampler row of data for the transfer to utilize
   * @param type The current execution flag, used for info printing
   * @param direction The direction of the transfer, used for info printing
   * @param verbose Whether or not print information about the transfer
   * @param console The console stream to output to
   */
  static void
  execBatchTransfers(const std::vector<std::shared_ptr<StochasticToolsTransfer>> & transfers,
                     dof_id_type global_row_index,
                     const std::vector<Real> & row_data,
                     Transfer::DIRECTION direction,
                     bool verbose,
                     const ConsoleStream & console);

protected:
  /// Override to avoid 'solve converged' message and print when processors are finished
  virtual void showStatusMessage(unsigned int i) const override;

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

  // Sampler size, to test if the MultiApp object needs to be re-initialize
  dof_id_type _number_of_sampler_rows;

  /// Current row of data updated by updateRowData. Used by transfers and setting command line args
  std::vector<Real> _row_data;
  /// Current local index representing _row_data
  dof_id_type _local_row_index = std::numeric_limits<dof_id_type>::max();

  /// Reporter value determining whether the sub-app should be run for a certain sample
  const std::vector<bool> * _should_run = nullptr;
};
