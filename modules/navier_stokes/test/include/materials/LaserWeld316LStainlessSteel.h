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
 */
class LaserWeld316LStainlessSteel : public Material
{
public:
  static InputParameters validParams();

  LaserWeld316LStainlessSteel(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /**
   * An approximate for the dynamic viscosity is provided in paper:
   * Kim, C.S., 1975. Thermophysical properties of stainless steels (No. ANL-75-55).
   * Argonne National Lab., Ill.(USA)., (needed unit conversion)
   */
  const Real _c_mu0;
  const Real _c_mu1;

  /**
   * The thermal conductivity is taken from:
   * Pichler, Peter, et al. "Surface tension and thermal conductivity of NIST SRM 1155a
   * (AISI 316L stainless steel)."
   * International Journal of Thermophysics 43.5 (2022): 66.
   */
  const Real _c_k0_s;
  const Real _c_k1_s;
  const Real _c_k2_s;
  const Real _c_k0_l;
  const Real _c_k1_l;
  const Real _c_k2_l;

  /**
   * An approximate for the speific heat is provided in paper:
   * Kim, C.S., 1975. Thermophysical properties of stainless steels (No. ANL-75-55).
   * Argonne National Lab., Ill.(USA)., (needed unit conversion)
   */
  const Real _c_cp0_s;
  const Real _c_cp1_s;
  const Real _c_cp0_l;

  /**
   * An approximate for the density is provided in paper:
   * Kim, C.S., 1975. Thermophysical properties of stainless steels (No. ANL-75-55).
   * Argonne National Lab., Ill.(USA)., (needed unit conversion)
   */
  const Real _c_rho0_s;
  const Real _c_rho1_s;
  const Real _c_rho2_s;
  const Real _c_rho0_l;
  const Real _c_rho1_l;
  const Real _c_rho2_l;

  /// Bounding temperature for the viscosity
  const Real _Tmax;

  /**
   * The liquidus and solidus temperatures, both taken from:
   * Pichler, Peter, et al. "Surface tension and thermal conductivity of NIST SRM 1155a
   * (AISI 316L stainless steel)."
   * International Journal of Thermophysics 43.5 (2022): 66.
   */
  const Real _Tl;
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

private:

  /// If a constant density should be used (would discard buoyancy effects)
  const bool _use_constant_density;
};
