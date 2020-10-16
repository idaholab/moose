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
class SamplerReceiver;

class OptimizationTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  OptimizationTransfer(const InputParameters & parameters);

  virtual void execute() override;

private:
  /**
   * Return the SamplerReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  SamplerReceiver * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const VectorPostprocessorName & _vpp_name;
  /// The name of the SamplerReceiver Control object on the sub-application
  const std::string & _receiver_name;
};
