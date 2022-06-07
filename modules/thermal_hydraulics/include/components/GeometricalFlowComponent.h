//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1D.h"
#include "GravityInterface.h"

/**
 * Base class for geometrical components that have fluid flow
 */
class GeometricalFlowComponent : public Component1D, public GravityInterface
{
public:
  GeometricalFlowComponent(const InputParameters & parameters);

  /**
   * Gets the gravity angle for this component
   *
   * @return gravity angle for this component
   */
  virtual const Real & getGravityAngle() const { return _gravity_angle; }

  /**
   * Gets the name of the fluid properties user object for this component
   */
  const UserObjectName & getFluidPropertiesName() const { return _fp_name; }

  /**
   * Gets the flow model ID
   */
  virtual const THM::FlowModelID & getFlowModelID() const = 0;

protected:
  /// Angle between orientation vector and gravity vector, in degrees
  const Real _gravity_angle;

  /// Name of fluid properties user object
  const UserObjectName & _fp_name;

public:
  static InputParameters validParams();
};
