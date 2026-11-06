//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPorosityLinear.h"

/**
 * Porosity (typically the aperture of a lower-dimensional fracture) given by the
 * PorousFlowPorosityLinear relationship plus a thermal-opening term that depends on how much
 * heat the surrounding matrix has gained or lost:
 *
 * porosity = linear terms - thermal_opening_coeff * I
 *
 * where I = int_0^infinity (T(z) - T_ref) dz is the heat content (in K.m) of the matrix
 * adjacent to the fracture.  I is computed using the semi-analytic method of
 * Vinsome and Westerveld (1980), which assumes the matrix temperature profile normal to the
 * fracture is
 *
 * T(z, t) - T_ref = (theta + p z + q z^2) exp(-z / d),   d = sqrt(kappa t) / 2
 *
 * with theta = T - T_ref the temperature change at the fracture face.  Each time step, p is
 * found from the energy balance and the heat-conduction equation at z = 0, giving
 *
 * p = (a theta / d + I_old - d^3 (theta - theta_old) / a) / (3 d^2 + a),   a = kappa dt
 * I = I_old + a (theta / d - p)
 *
 * so I is linear in the current temperature, and dI/dT is a scalar.
 *
 * If matrix_thickness = L is given, the matrix is a layer 0 <= z <= L with an insulated end at
 * z = L (for instance the rock between parallel fractures 2L apart), and the profile becomes
 *
 * T(z, t) - T_ref = P(z) exp(-z / d) + P(2L - z) exp(-(2L - z) / d),   P(z) = A + B z + C z^2
 *
 * which is symmetric about z = L.  A, B, C and the heat content I = int_0^L (T - T_ref) dz follow
 * from the surface value, the heat-conduction equation at z = 0 and the energy balance, a linear
 * system whose solution reduces to the expressions above as L -> infinity.  I is again linear in
 * the current temperature.
 */
class PorousFlowPorosityVinsomeWesterveld : public PorousFlowPorosityLinear
{
public:
  static InputParameters validParams();

  PorousFlowPorosityVinsomeWesterveld(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeUnboundedPorosityQp() override;

  /// Change of porosity per unit matrix heat content (units 1/K when porosity is an aperture)
  const Real _thermal_opening_coeff;

  /// Thermal diffusivity of the matrix in the direction normal to the fracture (m^2/s), assumed
  /// constant (independent of temperature and saturation)
  const Real _kappa;

  /// Undisturbed (reference) matrix temperature
  const VariableValue & _T_matrix_ref;

  /// Time at which the matrix temperature profile is undisturbed
  const Real _start_time;

  /// Whether only a layer of finite thickness next to the fracture contributes to the opening
  const bool _finite_thickness;

  /// Thickness of the matrix layer that contributes to the opening (used if _finite_thickness)
  const Real _matrix_thickness;

  /// Temperature change at the fracture face, theta = T - T_matrix_ref
  MaterialProperty<Real> & _theta;

  /// Old value of theta
  const MaterialProperty<Real> & _theta_old;

  /// Heat content of the adjacent matrix, I = int (T - T_matrix_ref) dz
  MaterialProperty<Real> & _heat_content;

  /// Old value of the heat content
  const MaterialProperty<Real> & _heat_content_old;
};
