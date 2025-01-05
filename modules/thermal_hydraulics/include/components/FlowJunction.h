//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1DJunction.h"

/**
 * Base class for flow junctions
 */
class FlowJunction : public Component1DJunction
{
public:
  FlowJunction(const InputParameters & params);

  /**
   * Gets the name of fluid properties used in all flow connections
   *
   * @return name of fluid properties used in all flow connections
   */
  const UserObjectName & getFluidPropertiesName() const;

  /// Get the name of the junction user object
  const std::string & getJunctionUOName() const { return _junction_uo_name; }

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Flow model ID
  THM::FlowModelID _flow_model_id;
  /// Flow model
  std::shared_ptr<const FlowModel> _flow_model;
  /// Fluid property user object name
  UserObjectName _fp_name;
  /// Name of junction user object name, if any
  const std::string _junction_uo_name;

public:
  static InputParameters validParams();
};
