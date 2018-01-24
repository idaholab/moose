//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SAMPLERRECEIVER_H
#define SAMPLERRECEIVER_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class SamplerReceiver;
class Function;

template <>
InputParameters validParams<SamplerReceiver>();

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class SamplerReceiver : public Control
{
public:
  SamplerReceiver(const InputParameters & parameters);
  virtual void execute() override;

protected:
  /**
   * Clears the list of parameters to modify
   */
  void reset();

  /**
   * Appends the list of parameters to modify
   */
  void addControlParameter(const std::string & name, const Real & value);

  /// Storage for the parameters to control
  std::map<std::string, Real> _parameters;

  /// Allows the SamplerTransfer to call the reset and addControlParameter methods, which
  /// should only be called by that object so making the public is dangerous.
  friend class SamplerTransfer;
};

#endif
