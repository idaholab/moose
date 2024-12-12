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
#include "TransientMultiApp.h"
#include "SamplerInterface.h"

#include "StochasticToolsTypes.h"

class Sampler;
class StochasticToolsTransfer;

class SamplerTransientMultiApp : public TransientMultiApp, public SamplerInterface
{
public:
  static InputParameters validParams();

  SamplerTransientMultiApp(const InputParameters & parameters);

  /**
   * Override solveStep to allow for batch execution.
   */
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  /**
   * Override to initialize batch backups.
   */
  virtual void initialSetup() override;

protected:
  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;

  /// The Sup-application solve mode
  const StochasticTools::MultiAppMode _mode;

  /// Counter for extracting command line arguments in batch mode
  dof_id_type _local_batch_app_index;

  /// Override to allow to get correct cli_args
  virtual std::vector<std::string> getCommandLineArgs(const unsigned int local_app) override;

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
   *
   * This is a copy from SamplerFullSolveMultiapp, but the alternative is to create an intermediate
   * base. But, given the inheritance difference between these object that gets rather complex, so
   * a few lines of copied code is better for now.
   */
  std::vector<std::shared_ptr<StochasticToolsTransfer>>
  getActiveStochasticToolsTransfers(Transfer::DIRECTION direction);

  /// Store the number of rows initialized, if this changes error because it doesn't make sense
  const dof_id_type _number_of_sampler_rows;
  /// Storage for batch-restore mode; the outer vector if for the local stochastic data and the
  /// inner vector is for the number of sub-apps. The later is 1 for this object, but it is included
  /// in case that changes in the future or in child classes
  std::vector<std::vector<std::unique_ptr<Backup>>> _batch_backup;

  /// Current row of data updated by updateRowData. Used by transfers and setting command line args
  std::vector<Real> _row_data;
  /// Current local index representing _row_data
  dof_id_type _local_row_index = std::numeric_limits<dof_id_type>::max();
};
