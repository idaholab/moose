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
#include "MultiAppTransfer.h"
#include "SamplerInterface.h"

class Sampler;

/**
 * The class creates an additional API to allow Transfers to work when running the
 * StochasticTools<FullSolve/Transient>MultiApp objects in batch-mode.
 */
class StochasticToolsTransfer : public MultiAppTransfer, SamplerInterface
{
public:
  static InputParameters validParams();

  StochasticToolsTransfer(const InputParameters & parameters);

  ///@{
  /**
   * Methods for transferring data from sub-applications to the master application.
   **/
  virtual void initializeFromMultiapp();
  virtual void executeFromMultiapp();
  virtual void finalizeFromMultiapp();
  ///@}

  ///@{
  /**
   * Methods for transferring data to sub-applications to the master application.
   **/
  virtual void initializeToMultiapp();
  virtual void executeToMultiapp();
  virtual void finalizeToMultiapp();
  ///@}

  /**
   * Method for setting the app index when running in batch mode.
   *
   * See StochasticTools<FullSolve/Transient>MultiApp
   */
  void setGlobalMultiAppIndex(dof_id_type index) { _app_index = index; }

  /**
   * Method for keeping track of the global row index when running in batch mode.
   *
   * See StochasticTools<FullSolve/Transient>MultiApp
   */
  void setGlobalRowIndex(dof_id_type row) { _global_index = row; }

  /**
   * Method for keeping track of the row data when running in batch mode.
   *
   * See StochasticTools<FullSolve/Transient>MultiApp
   */
  void setCurrentRow(const std::vector<Real> & row) { _row_data = row; }

protected:
  /// Index for the sub-app that the batch-mode multiapp is working on
  dof_id_type _app_index = 0;
  /// Index for tracking the row index when using batch mode operation
  dof_id_type _global_index = 0;
  /// The current row of data (comes from multiapp)
  std::vector<Real> _row_data;

  /// Pointer to the Sampler object used by the SamplerTransientMultiApp or SamplerFullSolveMultiApp
  Sampler * _sampler_ptr;
};
