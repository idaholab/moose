//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction1Phase.h"

/**
 * Gate valve component for 1-phase flow
 */
class GateValve1Phase : public FlowJunction1Phase
{
public:
  GateValve1Phase(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;

public:
  static InputParameters validParams();
};
