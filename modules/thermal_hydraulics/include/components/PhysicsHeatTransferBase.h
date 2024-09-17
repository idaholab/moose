//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatTransferBase.h"

/**
 * Base class for heat transfer connections to flow channels leveraging Physics
 */
class PhysicsHeatTransferBase : public HeatTransferBase
{
public:
  static InputParameters validParams();

  PhysicsHeatTransferBase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  /**
   * Returns wall heat transfer coefficient name
   *
   * @return The name of the wall heat transfer coefficient
   */
  const std::string & getWallHeatTransferCoefficientName() const;

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// Wall heat transfer coefficient name
  std::string _Hw_name;

  /// All the thermal hydraulics physics active on this heat transfer
  std::set<ThermalHydraulicsFlowPhysics *> _th_physics;
};
