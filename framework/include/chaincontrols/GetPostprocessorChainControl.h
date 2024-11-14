//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Copies a post-processor value into a ChainControlData.
 */
class GetPostprocessorChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  GetPostprocessorChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Chain control data
  Real & _value;

  /// Post-processor value
  const Real & _pp_value;
};
