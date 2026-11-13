//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpringLawBase.h"

/**
 * Interface spring law: local axis 1 is the normal, carrying compression and, while the bond
 * holds, tension up to the tensile strength; elastic shear along axes 2 and 3 capped by the
 * Coulomb limit cohesion + n tan(friction angle) for a compressive normal stress n, falling
 * to zero at the tensile strength. With permanent breakage a separation leaves the interface
 * frictional with the residual cohesion and friction angle and no tensile strength; without
 * it the bond is back when the interface closes again.
 */
class MohrCoulombSpring : public SpringLawBase
{
public:
  static InputParameters validParams();

  MohrCoulombSpring(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) override;

  /// Stiffnesses per unit area, normal and shear
  const Real _normal_stiffness;
  const Real _shear_stiffness;

  /// Strength parameters: cohesion (stress) and the tangent of the friction angle
  const Real _cohesion;
  const Real _tan_friction_angle;

  /// Compressive normal stress on the interface before any deformation
  const Real _initial_normal_stress;

  /// Tensile strength of the bond (stress); zero separates at zero normal stress
  const Real _tensile_strength;

  /// Whether a separation breaks the bond for good, and the strength parameters after it
  const bool _permanent_breakage;
  const Real _residual_cohesion;
  const Real _tan_residual_friction_angle;

  /// Plastic slip along axes 2 and 3 (component 0 unused)
  MaterialProperty<RealVectorValue> & _plastic_slip;
  const MaterialProperty<RealVectorValue> & _plastic_slip_old;

  /// 1 once the interface has separated (read only with permanent breakage)
  MaterialProperty<Real> & _bond_broken;
  const MaterialProperty<Real> & _bond_broken_old;
};
