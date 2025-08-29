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
 * Calculates the total power [W] that goes into the coolant based on
 * the thermal-hydraulic balance of inlet and outlet
 */
class SCMTHPowerPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SCMTHPowerPostprocessor(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  /// geometric information
  const SubChannelMesh & _mesh;
  /// value we want to calculate
  Real _value;
};
