//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowChannelBase.h"

class ClosuresBase;

/**
 * A class representing a flow channel leveraging a Physics for its equations
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class PhysicsFlowChannel : public FlowChannelBase
{
public:
  static InputParameters validParams();

  PhysicsFlowChannel(const InputParameters & params);

  virtual void addMooseObjects() override;

  /// Get the wall heat transfer coefficient
  virtual const std::vector<MaterialPropertyName> &
  getWallHeatTransferCoefficientNames() const override
  {
    return _Hw_names;
  }

  virtual const THM::FlowModelID & getFlowModelID() const override { return THM::FM_PHYSICS_BASED; }

  /// Return the Physics active on this flow channel
  const std::set<ThermalHydraulicsFlowPhysics *> & getPhysics() const { return _th_physics; }

protected:
  virtual void init() override;
  virtual std::shared_ptr<FlowModel> buildFlowModel() override { return nullptr; }
  virtual void check() const override;

  /**
   * Adds a material for the hydraulic diameter
   */
  virtual void addHydraulicDiameterMaterial();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames() override;

  /// Wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_names;

  /// Thermal hydraulics Physics active on this flow boundary
  std::set<ThermalHydraulicsFlowPhysics *> _th_physics;
};
