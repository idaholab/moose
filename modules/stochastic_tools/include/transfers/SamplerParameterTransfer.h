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
   * Return the SamplerReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  SamplerReceiver * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// The name of the SamplerReceiver Control object on the sub-application
  const std::string & _receiver_name;

  /// Current global index for batch execution
  dof_id_type _global_index;
};
