//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  SubChannelMesh & _mesh;
  const Real & _height;
  const int & _i_pin;
  Real _value;
};
