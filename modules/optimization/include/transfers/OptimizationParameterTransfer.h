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
class ControlsReceiver;

class OptimizationParameterTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  OptimizationParameterTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

private:
  /**
   * Return the ControlsReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  ControlsReceiver * getReceiver(unsigned int app_index);

  /// Storage for the list of parameters to control
  const VectorPostprocessorName & _vpp_name;
  /// The name of the ControlsReceiver Control object on the sub-application
  const std::string & _receiver_name;
};
