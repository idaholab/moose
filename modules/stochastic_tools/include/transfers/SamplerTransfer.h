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
#include "Sampler.h"

// Forward declarations
class SamplerTransfer;
class SamplerReceiver;

template <>
InputParameters validParams<SamplerTransfer>();

/**
 * Copy each row from each DenseMatrix to the sub-applications SamplerReceiver object.
 */
class SamplerTransfer : public StochasticToolsTransfer
{
public:
  SamplerTransfer(const InputParameters & parameters);
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
   * Return the SamplerReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  SamplerReceiver * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// Pointer to the Sampler object used by the SamplerTransientMultiApp or SamplerFullSolveMultiApp
  Sampler * _sampler_ptr;

  /// The name of the SamplerReceiver Control object on the sub-application
  const std::string & _receiver_name;

private:
  /// Storage for data returned from Sampler object
  std::vector<DenseMatrix<Real>> _samples;

  /// Current global index for batch execution
  dof_id_type _global_index;

  /**
   * Extract single row of Sampler data given the global index.
   */
  std::vector<Real> getRow(const dof_id_type global_index) const;
};
