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
#include "DerivativeMaterialInterface.h"

class SinglePhaseFluidProperties;

/**
 * Computes fluid properties in (P, T) formulation.
 */
class GeneralFluidProps : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();
  GeneralFluidProps(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const SinglePhaseFluidProperties & _fluid;

  /// Porosity
  const VariableValue & _eps;

  /**
   * Characteristic length $d$ used in computing the Reynolds number
   * $Re=\rho_fVd/\mu_f$.
   */
  const Real _d;

  /// variables
  const ADMaterialProperty<Real> & _pressure;
  const ADMaterialProperty<Real> & _T_fluid;
  const ADMaterialProperty<Real> & _rho;
  const ADMaterialProperty<Real> & _speed;

  /// Derivative of density with respect to pressure
  MaterialProperty<Real> & _drho_dp;

  /// Derivative of density with respect to temperature
  MaterialProperty<Real> & _drho_dT;

  /// Isobaric specific heat capacity
  ADMaterialProperty<Real> & _cp;

  /// Derivative of isobaric specific heat with respect to pressure
  MaterialProperty<Real> & _dcp_dp;

  /// Derivative of isobaric specific heat with respect to temperature
  MaterialProperty<Real> & _dcp_dT;

  /// Isochoric specific heat capacity
  ADMaterialProperty<Real> & _cv;

  /// Dynamic viscosity
  ADMaterialProperty<Real> & _mu;

  /// Derivative of dynamic viscosity with respect to pressure
  MaterialProperty<Real> & _dmu_dp;

  /// Derivative of dynamic viscosity with respect to temperature
  MaterialProperty<Real> & _dmu_dT;

  /// Thermal conductivity
  ADMaterialProperty<Real> & _k;

  /// Derivative of thermal conductivity with respect to pressure
  MaterialProperty<Real> & _dk_dp;

  /// Derivative of thermal conductivity with respect to temperature
  MaterialProperty<Real> & _dk_dT;

  /// Prandtl number
  ADMaterialProperty<Real> & _Pr;

  /**
   * Derivative of Prandtl number with respect to pressure.
   */
  MaterialProperty<Real> & _dPr_dp;

  /**
   * Derivative of Prandtl number with respect to temperature.
   */
  MaterialProperty<Real> & _dPr_dT;

  /// Pore (particle) Reynolds number
  ADMaterialProperty<Real> & _Re;

  /// Derivative of pore Reynolds number with respect to pressure
  MaterialProperty<Real> & _dRe_dp;

  /// Derivative of pore Reynolds number with respect to temperature
  MaterialProperty<Real> & _dRe_dT;

  /// Hydraulic Reynolds number
  ADMaterialProperty<Real> & _Re_h;

  /// Interstitial Reynolds number
  ADMaterialProperty<Real> & _Re_i;

  using UserObjectInterface::getUserObject;
  using DerivativeMaterialInterface<Material>::declarePropertyDerivative;
};
