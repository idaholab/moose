//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "DerivativeMaterialPropertyNameInterface.h"
#include "Function.h"

class SinglePhaseFluidProperties;
class Function;

/**
 * Computes fluid properties in (P, T) formulation using functor material properties
 */
class GeneralFunctorFluidProps : public FunctorMaterial, DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();
  GeneralFunctorFluidProps(const InputParameters & parameters);

protected:
  const SinglePhaseFluidProperties & _fluid;

  /// Porosity
  const Moose::Functor<ADReal> & _eps;

  /**
   * Characteristic length $d$ used in computing the Reynolds number
   * $Re=\rho_fVd/\mu_f$.
   */
  const Real _d;

  /// variables
  const Moose::Functor<ADReal> & _pressure;
  const Moose::Functor<ADReal> & _T_fluid;
  const Moose::Functor<ADReal> & _speed;

  /// Density as a material property, if density is not a variable
  FunctorMaterialProperty<ADReal> * const _rho_prop;

  /// Density as a functor, which could be from the variable set or the property
  const Moose::Functor<ADReal> & _rho;

  /// Derivative of density with respect to pressure
  FunctorMaterialProperty<Real> & _drho_dp;

  /// Derivative of density with respect to temperature
  FunctorMaterialProperty<Real> & _drho_dT;

  /// Derivative of density with respect to time
  FunctorMaterialProperty<ADReal> & _drho_dt;

  /// Isobaric specific heat capacity
  FunctorMaterialProperty<ADReal> & _cp;

  /// Derivative of isobaric specific heat with respect to pressure
  FunctorMaterialProperty<Real> & _dcp_dp;

  /// Derivative of isobaric specific heat with respect to temperature
  FunctorMaterialProperty<Real> & _dcp_dT;

  /// Derivative of isobaric specific heat with respect to time
  FunctorMaterialProperty<ADReal> & _dcp_dt;

  /// Isochoric specific heat capacity
  FunctorMaterialProperty<ADReal> & _cv;

  /// Dynamic viscosity
  FunctorMaterialProperty<ADReal> & _mu;

  /// Derivative of dynamic viscosity with respect to pressure
  FunctorMaterialProperty<Real> & _dmu_dp;

  /// Derivative of dynamic viscosity with respect to temperature
  FunctorMaterialProperty<Real> & _dmu_dT;

  /// Function to ramp down the viscosity, useful for relaxation transient
  const Function & _mu_rampdown;

  /// Thermal conductivity
  FunctorMaterialProperty<ADReal> & _k;

  /// Derivative of thermal conductivity with respect to pressure
  FunctorMaterialProperty<Real> & _dk_dp;

  /// Derivative of thermal conductivity with respect to temperature
  FunctorMaterialProperty<Real> & _dk_dT;

  /// Prandtl number
  FunctorMaterialProperty<ADReal> & _Pr;

  /**
   * Derivative of Prandtl number with respect to pressure.
   */
  FunctorMaterialProperty<Real> & _dPr_dp;

  /**
   * Derivative of Prandtl number with respect to temperature.
   */
  FunctorMaterialProperty<Real> & _dPr_dT;

  /// Pore (particle) Reynolds number
  FunctorMaterialProperty<ADReal> & _Re;

  /// Derivative of pore Reynolds number with respect to pressure
  FunctorMaterialProperty<Real> & _dRe_dp;

  /// Derivative of pore Reynolds number with respect to temperature
  FunctorMaterialProperty<Real> & _dRe_dT;

  /// Hydraulic Reynolds number
  FunctorMaterialProperty<ADReal> & _Re_h;

  /// Interstitial Reynolds number
  FunctorMaterialProperty<ADReal> & _Re_i;

  using DerivativeMaterialPropertyNameInterface::derivativePropertyNameFirst;
  using UserObjectInterface::getUserObject;
};
