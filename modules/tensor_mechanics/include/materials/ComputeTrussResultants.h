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
#include "RankTwoTensor.h"

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

  const MaterialProperty<Real> & _elastic_stretch;

  /// Mechanical displacement strain increment in truss local coordinate system
  // const MaterialProperty<RealVectorValue> & _disp_strain_increment;

  /// Material stiffness vector that relates displacement strain increment to force increment
  const MaterialProperty<Real> & _material_stiffness;

  // /// Material flexure vector that relates rotational strain increment to moment increment
  // const MaterialProperty<RealVectorValue> & _material_flexure;

  /// Rotational transformation from global to current truss local coordinate system
  // const MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current force vector in global coordinate system
  // MaterialProperty<RealVectorValue> & _force;
  MaterialProperty<Real> & _force;

  // /// Current moment vector in global coordinate system
  // MaterialProperty<RealVectorValue> & _moment;

  /// Old force vector in global coordinate system
  // const MaterialProperty<RealVectorValue> & _force_old;
  const MaterialProperty<Real> & _force_old;

  // /// Old force vector in global coordinate system
  // const MaterialProperty<RealVectorValue> & _moment_old;
};
