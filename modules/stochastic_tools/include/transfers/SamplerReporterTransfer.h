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
#include "ReporterTransferInterface.h"

// Need forward declration for some reason
class StochasticReporter;

/**
 * Transfer Reporters from sub-applications to a StochasticReporter on the main application.
 *
 * This object transfers the distributed data to a StochasticReporter object.
 */
class SamplerReporterTransfer : public StochasticToolsTransfer, public ReporterTransferInterface
{
public:
  static InputParameters validParams();
  SamplerReporterTransfer(const InputParameters & parameters);
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

  /**
   * Used to declare reporter values on main app and add consumer modes on subapps
   */
  void intitializeStochasticReporters();

  /**
   * Transfer reporter values
   * @param global_index The global row of the sampler matrix
   * @param app_index The subapp index to transfer from
   */
  void transferStochasticReporters(dof_id_type global_index, dof_id_type app_index);

  /// Name of reporters on the sub-applications
  const std::vector<ReporterName> & _sub_reporter_names;

  /// StochasticReporter object where values are being transferred
  StochasticReporter * _results = nullptr;

  /// Storage vector names
  std::vector<ReporterName> _reporter_names;

  /// Reporter value for whether or not sub app converged
  std::vector<bool> * _converged;
};
