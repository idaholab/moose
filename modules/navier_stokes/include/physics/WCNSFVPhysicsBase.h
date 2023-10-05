//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesFlowPhysics.h"

/**
 * Base class to hold common parameters and utilities between all the weakly compressible
 * Navier Stokes-based equations
 * Includes incompressible flow (INSFV).
 */
class WCNSFVPhysicsBase : public NavierStokesFlowPhysics
{
public:
  static InputParameters validParams();

  WCNSFVPhysicsBase(const InputParameters & parameters);

  /// GeneralUO not the right base class probably
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

protected:
};
