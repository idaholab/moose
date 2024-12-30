//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsHeatTransferBase.h"

/**
 * Base class for heat transfer connections from temperature for components using Physics
 */
class PhysicsHeatTransferFromHeatFlux : public PhysicsHeatTransferBase
{
public:
  static InputParameters validParams();

  PhysicsHeatTransferFromHeatFlux(const InputParameters & parameters);

  void init() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

  const MooseFunctorName getWallHeatFluxFunctorName() const override;
};
