//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

/**
 *  Material class used to provide the electric field as a material property and computes
 *  the residual contributions for electromagnetic/electrostatic heating objects.
 */
class ElectromagneticHeatingMaterial : public ADMaterial
{
public:
  static InputParameters validParams();

  ElectromagneticHeatingMaterial(const InputParameters & parameters);

  virtual void computeQpProperties() override;
  /// Function that defines the field depending on supplied variable type.
  virtual void computeFieldValue();
  /// Function that defines the residual for Joule heating
  virtual void computeJouleHeating();

protected:
  /// The variable data of the supplied variable for the electric field
  const MooseVariableFieldBase & _electric_field_var;
  /// True if the supplied variable is a vector
  const bool _is_vector;
  /// The electric field defined from a vector variable
  const ADVectorVariableValue & _efield;
  /// The complex component of the electric field, needed for time-harmonic formulations
  const ADVectorVariableValue & _efield_complex;
  /// The electric field defined from the gradient of a scalar variable
  const ADVariableGradient & _grad_potential;
  /// Electric field material property
  ADMaterialProperty<RealVectorValue> & _electric_field;
  /// Complex electric field material property
  ADMaterialProperty<RealVectorValue> & _electric_field_complex;
  /// Joule heating residual material property
  ADMaterialProperty<Real> & _electric_field_heating;
  /// Coefficient to multiply by heating term
  const Real & _heating_scaling;
  /// Real component of the material conductivity (in S/m)
  const ADMaterialProperty<Real> & _elec_cond;
  /// The domain formulation of the EM residuals (either TIME or FREQUENCY)
  MooseEnum _formulation;
  /// The solver formulation the electric field (either ELECTROSTATIC or ELECTROMAGNETIC)
  MooseEnum _solver;
};
