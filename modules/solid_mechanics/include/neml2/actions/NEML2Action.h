//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#endif

#include "Action.h"

/**
 * Action to parse and set up NEML2 objects.
 */
class NEML2Action : public Action
{
public:
  static InputParameters validParams();

  NEML2Action(const InputParameters & params);

  virtual void act() override;

#ifdef NEML2_ENABLED

protected:
  /// Name of the NEML2 input file
  FileName _fname;

  /// Name of the NEML2 material model to import from the NEML2 input file
  std::string _mname;

  /// Whether to print additional information about the NEML2 material model
  const bool _verbose;

  /// The operation mode
  const MooseEnum _mode;

  /// The device on which to evaluate the NEML2 model
  const torch::Device _device;

  /// Whether AD is enabled
  const bool _enable_AD;
#endif
};
