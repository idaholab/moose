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
#include "ParameterReceiver.h"

// Forward declarations
class Function;

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class SamplerReceiver : public ParameterReceiver
{
public:
  static InputParameters validParams();
  SamplerReceiver(const InputParameters & parameters);
};
