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
#include "ParameterReceiverInterface.h"

// Forward declarations
class ParameterReceiver;

/**
 * Copy each row from each DenseMatrix to the sub-applications ParameterReceiver object.
 */
class SamplerParameterTransfer : public StochasticToolsTransfer, public ParameterReceiverInterface
{
public:
  static InputParameters validParams();

  SamplerParameterTransfer(const InputParameters & parameters);
  /**
   * Traditional Transfer callback
   */
  virtual void execute() override;

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeToMultiapp() override;
  virtual void executeToMultiapp() override;
  virtual void finalizeToMultiapp() override;
  ///@}

protected:
  /**
   * Return the ParameterReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  ParameterReceiver * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// The name of the ParameterReceiver Control object on the sub-application
  const std::string & _receiver_name;

  /// Current global index for batch execution
  dof_id_type _global_index;
};
