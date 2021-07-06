//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * ComputeTrussResultants computes forces and moments using elasticity
 */
class ComputeTrussResultants : public Material
{
public:
  static InputParameters validParams();

  ComputeTrussResultants(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Coupled variable for the beam cross-sectional area
  const VariableValue & _area;

  /// Mechanical displacement strain increment in truss local coordinate system
  const MaterialProperty<RealVectorValue> & _disp_strain_increment;

  /// Material stiffness vector that relates displacement strain increment to force increment
  const MaterialProperty<Real> & _material_stiffness;

  MaterialProperty<Real> & _axial_stress;

  const MaterialProperty<Real> & _axial_stress_old;

  /// Current force vector in global coordinate system
  MaterialProperty<Real> & _force;

  /// Old force vector in global coordinate system
  const MaterialProperty<Real> & _force_old;
};
