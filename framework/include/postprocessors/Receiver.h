//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * A class for storing data, it allows the user to change the value of the
 * postprocessor by altering the _my_value reference
 */
class Receiver : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  Receiver(const InputParameters & parameters);

  ///@{
  /**
   * No action taken
   */
  virtual void initialize() override {}
  virtual void execute() override {}
  ///@}

  /**
   * Returns the value stored in _my_value
   * @return A const reference to the value of the postprocessor
   */
  virtual Real getValue() override;

private:
  /// Flag for initializing the old value
  bool _initialize_old;

  /// Reference to the value being stored in the associated PostprocessorData class
  const PostprocessorValue & _my_value;
};
