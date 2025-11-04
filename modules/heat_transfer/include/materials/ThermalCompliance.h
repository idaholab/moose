//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "MathUtils.h"

/**
 * Computes heat conduction compliance.
 * This material can be used to monitor the thermal
 * compliance of a system that is being optimized.
 */
class ThermalCompliance : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ThermalCompliance(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Temperature gradient vector
  const VariableGradient & _grad_temperature;

  /// Thermal conductivity material (parsed)
  const MaterialProperty<Real> & _thermal_conductivity;

  /// Generated thermal compliance material
  MaterialProperty<Real> & _thermal_compliance;
};
