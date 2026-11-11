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
 * Calculates the heat flow $[W]$ that axial conduction carries out of the assembly through the
 * inlet boundary, against the flow. The inlet enthalpy is prescribed, so when the first cell is
 * heated the enthalpy gradient across the inlet face drives this flux. It is the term that closes
 * the assembly energy balance between the pin and duct heat rates and the enthalpy flow rate rise
 * of SCMTHPowerPostprocessor.
 */
class SCMInletConductionHeatRatePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SCMInletConductionHeatRatePostprocessor(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

protected:
  /// geometric information
  const SubChannelMesh & _mesh;
  /// heat rate conducted out of the inlet [W]
  Real _value;
};
