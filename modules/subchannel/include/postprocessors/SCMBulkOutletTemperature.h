//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralPostprocessor.h"
#include "SubChannelMesh.h"

class SinglePhaseFluidProperties;

/**
 * Calculates the bulk outlet temperature from the mass-flow-weighted outlet
 * enthalpy of a subchannel assembly.
 */
class SCMBulkOutletTemperature : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SCMBulkOutletTemperature(const InputParameters & params);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  /// Geometric information
  const SubChannelMesh & _mesh;
  /// Outlet pressure used by the fluid-property inversion
  const Real & _pressure;
  /// Fluid-property model used by SCM
  const SinglePhaseFluidProperties & _fp;
  /// Bulk outlet temperature
  Real _value;
};
