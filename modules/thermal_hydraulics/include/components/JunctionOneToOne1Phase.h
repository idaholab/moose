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
 * Junction connecting one flow channel to one other flow channel for 1-phase flow
 *
 * The assumptions made by this component are as follows:
 * @li The connected channels are parallel.
 * @li The connected channels have the same flow area at the junction.
 */
class JunctionOneToOne1Phase : public FlowJunction1Phase
{
public:
  JunctionOneToOne1Phase(const InputParameters & params);

  virtual void init() override;
  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;

  /// Slope reconstruction option
  MooseEnum _slope_reconstruction;

public:
  static InputParameters validParams();
};
