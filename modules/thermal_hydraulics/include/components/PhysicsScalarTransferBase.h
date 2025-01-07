//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarTransferBase.h"

/**
 * Base class for scalar transfer connections to flow channels leveraging Physics
 */
class PhysicsScalarTransferBase : public ScalarTransferBase
{
public:
  static InputParameters validParams();

  PhysicsScalarTransferBase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  /**
   * Returns wall heat transfer coefficient name
   *
   * @return The name of the wall heat transfer coefficient
   */
  const std::string & getWallScalarTransferCoefficientName(unsigned int scalar_i) const;

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// Wall scalar transfer coefficient name
  std::vector<std::string> _wall_scalar_H_names;

  /// All the thermal hydraulics physics active on this heat transfer
  std::set<ThermalHydraulicsFlowPhysics *> _th_physics;
};
