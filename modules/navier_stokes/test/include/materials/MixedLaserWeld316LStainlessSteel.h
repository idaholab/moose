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
 * A material that computes 316L volumetric stainless steel properties relevant to doing laser
 * welding modeling.
 * The following thermophysical properties have been taken
 * Kim, C.S., 1975. Thermophysical properties of stainless steels (No. ANL-75-55).
 * Argonne National Lab., Ill.(USA).
 *
 * Certain other information is added (for melting point) from:
 * Pichler, Peter, et al. "Surface tension and thermal conductivity of NIST SRM 1155a
 * (AISI 316L stainless steel)."
 * International Journal of Thermophysics 43.5 (2022): 66.
 */
class MixedLaserWeld316LStainlessSteel : public Material
{
public:
  static InputParameters validParams();

  MixedLaserWeld316LStainlessSteel(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /**
   * An approximate for the dynamic viscosity is provided in the paper, which is
   * given in [cp] and therefore needs to be converted to [Pa*s]
   * 1E-3*exp(2385.2/T[K]-0.5958)
   */
  const Real _c_mu0;
  const Real _c_mu1;

  /**
   * Thermal conductivities are approximated as linear functions. The
   * reference provides these in [W/cm/K] so we convert here to [W/m/K]:
   * Solid: 9.248+1.571E-2*T[K]
   * Liquid: 12.41+3.279E-3*T[K]
   */
  const Real _c_k0_s;
  const Real _c_k1_s;
  const Real _c_k0_l;
  const Real _c_k1_l;

  /**
   * Specific heat is a linear function of temperature. the original reference gave it
   * in [cal/g/K] which is 4184 [J/kg/K]:
   * Solid: 458.98 + 0.1328*T[K]
   * Liquid: 794.96
   */
  const Real _c_cp0_s;
  const Real _c_cp1_s;
  const Real _c_cp0_l;

  /**
   * Density will be a quadratic function, but the function will depend on if the
   * steel is liquid of solid. The values in the reference are provided [g/cm3], so
   * we convert them here to [kg/m3]:
   * Solid: 8084.2-0.42086*T[K]-3.8942E-5*T^2[K]
   * Liquid: 7432.7+0.039338*T[K]-1.8007E-4*T^2[K]
   */
  const Real _c_rho0_s;
  const Real _c_rho1_s;
  const Real _c_rho2_s;
  const Real _c_rho0_l;
  const Real _c_rho1_l;
  const Real _c_rho2_l;

  /// Bounding temperature for the
  const Real _Tmax;

  /// Liquidus temperature
  const Real _Tl;

  /// Solidus temperature
  const Real _Ts;

  /// We need to know the temperature and the gradient of the temperature
  const ADVariableValue & _temperature;
  const ADVariableGradient & _grad_temperature;

  /// Material properties that get updated in this object
  ADMaterialProperty<Real> & _mu;
  ADMaterialProperty<Real> & _k;
  ADMaterialProperty<Real> & _cp;
  ADMaterialProperty<Real> & _rho;

  /// The gradient of the thermal conductivity
  ADMaterialProperty<RealVectorValue> & _grad_k;
};
