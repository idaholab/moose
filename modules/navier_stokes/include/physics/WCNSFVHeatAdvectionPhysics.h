//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVPhysicsBase.h"

/**
 * Creates all the objects needed to solve the Navier Stokes energy equation
 */
class WCNSFVHeatAdvectionPhysics : public WCNSFVPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVHeatAdvectionPhysics(const InputParameters & parameters);

  /// GeneralUO not the right base class probably
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

protected:
private:
};
