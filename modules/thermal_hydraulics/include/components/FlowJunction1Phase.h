//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction.h"

/**
 * Base class for 1-phase flow junctions
 */
class FlowJunction1Phase : public FlowJunction
{
public:
  FlowJunction1Phase(const InputParameters & params);

protected:
  virtual void init() override;
  virtual void check() const override;

  // Numerical flux user object names
  std::vector<UserObjectName> _numerical_flux_names;

public:
  static InputParameters validParams();
};
