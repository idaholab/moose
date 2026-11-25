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
#include "RankTwoTensor.h"

/**
 * Base class of the spring laws: a force and a tangent stiffness from the relative
 * deformation computed by ComputeSpringDeformation.
 *
 * A law acts either on the three relative translations (mode = translational, reporting
 * spring_force and spring_stiffness) or on the three relative rotations (mode = rotational,
 * reporting spring_moment and spring_rotational_stiffness). Every force is multiplied by the
 * tributary area, so that the law coefficients can be given per unit area of the interface
 * the spring represents.
 */
class SpringLawBase : public Material
{
public:
  static InputParameters validParams();

  SpringLawBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /**
   * The law: force (or moment) and tangent stiffness per unit tributary area from the
   * deformation (or rotation) in the local frame at the current quadrature point.
   */
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) = 0;

  /// Prefix of the property names, the base_name of the deformation material of this spring
  const std::string _base_name;

  /// Whether the law acts on the rotations
  const bool _rotational;

  /// Multiplier of every force
  const Real _tributary_area;

  /// Relative deformation (translational) or rotation (rotational) in the local frame
  const MaterialProperty<RealVectorValue> & _deformation;

  /// Force (moment) and tangent stiffness in the local frame
  MaterialProperty<RealVectorValue> & _force;
  MaterialProperty<RankTwoTensor> & _stiffness;
};
