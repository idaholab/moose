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
#include "GeneralPostprocessor.h"

class RealControlParameterReporter : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RealControlParameterReporter(const InputParameters & parameters);

  /**
   * Extract the parameter via the ControlInterface::getControlParam
   **/
  virtual void initialSetup();

  ///@{
  /**
   * These methods left intentionally empty
   */
  virtual void initialize() {}
  virtual void execute() {}
  ///@}

  /**
   * Return the parameter value
   */
  virtual Real getValue();

private:
  // Pointer to the parameter to report, a pointer is used because the access
  // of the parameter value must occur in initialSetup because all objects
  // must be created prior to attempting to access the parameter objects
  const Real * _parameter;
};
