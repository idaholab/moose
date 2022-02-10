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
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 */
class FreeBoundary1Phase : public FlowBoundary1Phase
{
public:
  FreeBoundary1Phase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
