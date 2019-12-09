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
#include "SamplerInterface.h"

// Forward declarations
class MultiAppCommandLineControl;

class Function;
class Sampler;
class MultiApp;

template <>
InputParameters validParams<MultiAppCommandLineControl>();

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class MultiAppCommandLineControl : public Control, public SamplerInterface
{
public:
  MultiAppCommandLineControl(const InputParameters & parameters);

  /**
   * Do not allow the use of initialSetup, because this class is designed to operate
   * on PRE_MULTIAPP_SETUP, which occurs before this callback. This will prevent a child class
   * adding something to this function without it doing anything.
   */
  virtual void initialSetup() override final;

  virtual void execute() override;

protected:
  /// The MultiApp this Transfer is transferring data to or from
  std::shared_ptr<MultiApp> _multi_app;

  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;

  /// Storage for the parameter names to be applied
  const std::vector<std::string> & _param_names;
};

