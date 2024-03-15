//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Component for testing Logger
 */
class LoggerTestComponent : public Component
{
public:
  LoggerTestComponent(const InputParameters & params);

private:
  const bool _log_errors;
  const bool _log_warnings;

public:
  static InputParameters validParams();
};
