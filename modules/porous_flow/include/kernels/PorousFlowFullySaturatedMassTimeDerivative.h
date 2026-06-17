//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "PorousFlowDictator.h"

/**
 * Time derivative of fluid mass for fully-saturated, single-phase,
 * single-component simulations.  No mass-lumping is used.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties
 * and requires no hand-coded Jacobian.
 */
template <bool is_ad>
class PorousFlowFullySaturatedMassTimeDerivativeTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedMassTimeDerivativeTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Jacobian contribution for PorousFlow variable pvar (non-AD path only)
  Real computeQpJac(unsigned int pvar);

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Whether the kernel variable is a PorousFlow variable
  const bool _var_is_porflow_var;

  /// If true the kernel is d(fluid mass)/dt, otherwise d(fluid volume)/dt
  const bool _multiply_by_density;

  enum class CouplingTypeEnum
  {
    Hydro,
    ThermoHydro,
    HydroMechanical,
    ThermoHydroMechanical
  };
  const CouplingTypeEnum _coupling_type;

  const bool _includes_thermal;
  const bool _includes_mechanical;

  /// Biot coefficient
  const Real _biot_coefficient;

  /// Constant Biot modulus (not a function of primary variables)
  const MaterialProperty<Real> & _biot_modulus;

  /// Constant volumetric thermal expansion coefficient
  const MaterialProperty<Real> * const _thermal_coeff;

  /// Quadpoint fluid density (AD or non-AD depending on is_ad)
  const GenericMaterialProperty<std::vector<Real>, is_ad> * const _fluid_density;

  /// d(fluid density)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;

  /// Quadpoint pore pressure
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _pp;

  /// Old quadpoint pore pressure (always non-AD)
  const MaterialProperty<std::vector<Real>> & _pp_old;

  /// d(pore pressure)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dpp_dvar;

  /// Quadpoint temperature (only when _includes_thermal)
  const GenericMaterialProperty<Real, is_ad> * const _temperature;

  /// Old quadpoint temperature (always non-AD)
  const MaterialProperty<Real> * const _temperature_old;

  /// d(temperature)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;

  /// Volumetric strain rate (only when _includes_mechanical; always non-AD)
  const MaterialProperty<Real> * const _strain_rate;

  /// d(strain rate)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<RealGradient>> * const _dstrain_rate_dvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowFullySaturatedMassTimeDerivativeTempl<false>
    PorousFlowFullySaturatedMassTimeDerivative;
typedef PorousFlowFullySaturatedMassTimeDerivativeTempl<true>
    ADPorousFlowFullySaturatedMassTimeDerivative;
