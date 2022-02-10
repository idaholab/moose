//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Applies a specified heat flux to a heat structure boundary
 */
class HSBoundaryHeatFlux : public HSBoundary
{
public:
  HSBoundaryHeatFlux(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// Heat flux function name
  const FunctionName & _q_fn_name;

public:
  static InputParameters validParams();
};
