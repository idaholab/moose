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
 * Base class for creating component that connect other components together (e.g. a flow channel and
 * a heat structure)
 */
class ConnectorBase : public Component
{
public:
  ConnectorBase(const InputParameters & params);

public:
  static InputParameters validParams();
};
