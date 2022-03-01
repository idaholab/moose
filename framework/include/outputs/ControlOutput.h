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
#include "Output.h"

/**
 * Class for output information regarding Controls to the screen
 */
class ControlOutput : public Output
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  ControlOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the output of control information
   */
  virtual void output(const ExecFlagType & type) override;

private:
  /**
   * Output a list of active MooseObjects
   */
  void outputActiveObjects();

  /**
   * Output list of controllable parameters
   */
  void outputControls();

  /**
   * Output list of parameters that have been controlled
   */
  void outputChangedControls();

  /// Flag for clearing the controlled parameters after they are output
  bool _clear_after_output;

  /// Flag for showing active objects
  bool _show_active_objects;
};
