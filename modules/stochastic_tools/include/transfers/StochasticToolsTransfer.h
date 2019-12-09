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

class StochasticToolsTransfer;
template <>
InputParameters validParams<StochasticToolsTransfer>();

/**
 * The class creates an additional API to allow Transfers to work when running the
 * StochasticTools<FullSolve/Transient>MultiApp objects in batch-mode.
 */
class StochasticToolsTransfer : public MultiAppTransfer
{
public:
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
   * Method for keeping track of the global MultiApp index when running in batch mode.
   *
   * See StochasticTools<FullSolve/Transient>MultiApp
   */
  void setGlobalMultiAppIndex(dof_id_type index) { _global_index = index; }

protected:
  /// Index for tracking the global index when using batch mode operation
  dof_id_type _global_index = 0;
};
