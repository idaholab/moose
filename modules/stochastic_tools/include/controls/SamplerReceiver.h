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
class SamplerReceiver : public Control
{
public:
  static InputParameters validParams();

  SamplerReceiver(const InputParameters & parameters);
  virtual void execute() override;

protected:
  /**
   * Update the parameters and associated values via _parameters and _values.
   *
   * @param param_values Map between the parameter name and its value. Real-type parameters will
   * have a single value.
   */
  void transfer(const std::map<std::string, std::vector<Real>> & param_values);

  /// Parameter names to modify
  std::vector<std::string> _parameters;

  /// Values to use when modifying parameters
  std::vector<std::vector<Real>> _values;

  /// Allows the SamplerParameterTransfer to call the transfer method, which
  /// should only be called by that object so making it public is dangerous.
  friend class SamplerParameterTransfer;
};
