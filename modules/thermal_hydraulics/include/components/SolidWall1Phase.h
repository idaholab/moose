//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Component for solid wall BC for 1-phase flow
 */
class SolidWall1Phase : public FlowBoundary1Phase
{
public:
  SolidWall1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
