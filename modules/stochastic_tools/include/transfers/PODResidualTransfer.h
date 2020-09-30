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
#include "PODSamplerSolutionTransfer.h"

/**
 * Transfers residuals for given variables and vector tags from a sub-subapplication
 * to a PODReducedBasisTrainer object.
 */
class PODResidualTransfer : public PODSamplerSolutionTransfer
{
public:
  static InputParameters validParams();
  PODResidualTransfer(const InputParameters & parameters);

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeFromMultiapp() override{};
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override{};
  ///@}

protected:
  /**
   * Transfer callback that will transfer residuals with given tags from the
   * subapplication.
   */
  virtual void execute() override;

private:
  /**
   * Adds the variable-residuals to the trainer.
   */
  void transferResidual(dof_id_type base_i, dof_id_type multi_app_i);
};
