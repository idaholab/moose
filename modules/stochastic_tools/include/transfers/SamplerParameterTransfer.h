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
class SamplerReceiver;

/**
 * Copy each row from each DenseMatrix to the sub-applications SamplerReceiver object.
 */
class SamplerParameterTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();

  SamplerParameterTransfer(const InputParameters & parameters);
  /**
   * Traditional Transfer callback
   */
  virtual void execute() override;

  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void executeToMultiapp() override;

protected:
  /**
   * Based on command line args, return a map between SamplerReceiver objects and the
   * parameter-value pairs.
   *
   * @param app_index The global sup-app index
   * @param args The command line args
   * @return A map between the SamplerReceiver object and the parameter-value pairs
   */
  std::map<SamplerReceiver *, std::map<std::string, std::vector<Real>>>
  getReceivers(unsigned int app_index, const std::vector<std::string> & args);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// Current global index for batch execution
  dof_id_type _global_index;

private:
  /**
   * Helper function that recursively finds feproblem pointers from nested multiapps
   */
  static std::vector<FEProblemBase *>
  getMultiAppProblemsHelper(FEProblemBase & base_problem,
                            const std::vector<std::string> & multiapp_names);
};
