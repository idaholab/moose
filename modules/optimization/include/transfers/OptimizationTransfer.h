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

// Forward declarations
class SamplerReceiver2;

class OptimizationTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  OptimizationTransfer(const InputParameters & parameters);
  /**
   * Traditional Transfer callback
   */
  virtual void execute() override;
  ///@{
  /**
   * Methods for transferring data from sub-applications to the master application.
   **/
  //  void initializeFromMultiapp();
  //  void executeFromMultiapp();
  //  void finalizeFromMultiapp();
  ///@}

  ///@{
  /**
   * Methods for transferring data to sub-applications to the master application.
   **/
  void executeToMultiapp();
  ///@}

private:
  /**
   * Return the SamplerReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  SamplerReceiver2 * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// Initial guesses for parameter values
  const std::vector<Real> & _guess_values;

  /// The name of the SamplerReceiver Control object on the sub-application
  const std::string & _receiver_name;
};
