//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "SubChannelMesh.h"

/**
 * Returns the surface temperature of a specific fuel pin at a user defined height
 */
class SCMPinSurfaceTemperature : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SCMPinSurfaceTemperature(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  /// geometric information
  const SubChannelMesh & _mesh;
  /// axial location [m]
  const Real & _height;
  /// pin index
  const int & _i_pin;
  /// value we want to calculate
  Real _value;
};
