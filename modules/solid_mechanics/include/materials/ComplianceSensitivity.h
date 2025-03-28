//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StrainEnergyDensity.h"
#include "MathUtils.h"

/**
 * Computes the elasticity compliance sensitivity with respect to a user-supplied variable.
 */
class ComplianceSensitivity : public StrainEnergyDensity
{
public:
  static InputParameters validParams();

  ComplianceSensitivity(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Sensitivity material generated by this error
  MaterialProperty<Real> & _sensitivity;
  /// Pseudo-density variable
  const VariableValue & _design_density;
  /// Pseudo-density variable name
  const MaterialPropertyName _design_density_name;
  /// Derivative of elasticity modulus with respect to pseudo-density
  const MaterialProperty<Real> & _dEdp;
  /// Young's modulus of the material
  const MaterialProperty<Real> & _youngs_modulus;
};
