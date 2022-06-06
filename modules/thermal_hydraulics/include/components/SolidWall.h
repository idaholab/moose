//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary.h"

/**
 * A simple component for solid wall BC
 *
 * Deprecated
 */
class SolidWall : public FlowBoundary
{
public:
  SolidWall(const InputParameters & params);

public:
  static InputParameters validParams();
};
