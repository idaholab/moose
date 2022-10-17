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
 * 1-phase inlet with all variables prescribed by functions.
 */
class InletFunction1Phase : public FlowBoundary1Phase
{
public:
  InletFunction1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
