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
#include "Control.h"

// Forward declarations
class Function;

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class SamplerReceiver2 : public Control
{
public:
  static InputParameters validParams();

  SamplerReceiver2(const InputParameters & parameters);
  virtual void execute() override;

protected:
  /**
   * Update the parameter names and associated values.
   */
  void transfer(const std::vector<std::string> & names, const std::vector<Real> & values);

  /// Parameter names to modify
  std::vector<std::string> _parameters;

  /// Values to use when modifying parameters
  std::vector<Real> _values;

  /// Allows the SamplerParameterTransfer to call the transfer method, which
  /// should only be called by that object so making it public is dangerous.
  friend class SamplerParameterTransfer;
  friend class OptimizationTransfer;
};
