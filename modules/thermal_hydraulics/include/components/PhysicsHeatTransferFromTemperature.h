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
class PhysicsHeatTransferFromTemperature : public PhysicsHeatTransferBase
{
public:
  static InputParameters validParams();

  PhysicsHeatTransferFromTemperature(const InputParameters & parameters);

  void init() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /// Get the FE type for wall temperature variable
  virtual const FEType & getFEType();

  /// The type of the wall temperature variable
  const FEType _fe_type;
};
