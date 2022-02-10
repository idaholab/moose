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
 *
 */
class SupersonicInlet : public FlowBoundary1Phase
{
public:
  SupersonicInlet(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
