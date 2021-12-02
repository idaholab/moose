//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPorosityBase.h"

/**
 * Material designed to provide the porosity in PorousFlow simulations
 * porosity_ref + P_coeff * (P - P_ref) + T_coeff * (T - T_ref) + epv_coeff * (epv - epv_ref), where
 * P is the effective porepressure, T is the temperature and epv is the volumetric strain
 */
class PorousFlowPorosityLinear : public PorousFlowPorosityBase
{
public:
  static InputParameters validParams();

  PorousFlowPorosityLinear(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// When calculating nodal porosity, use the strain at the nearest quadpoint to the node
  const bool _strain_at_nearest_qp;

  /// If the linear relationship produces porosity < _porosity_min, then porosity is set to _porosity_min
  const Real _porosity_min;

  /// Porosity at reference porepressure, temperature and volumetric strain
  const VariableValue & _phi_ref;

  /// Reference effective porepressure
  const VariableValue & _P_ref;

  /// Reference temperature
  const VariableValue & _T_ref;

  /// Reference volumetric strain
  const VariableValue & _epv_ref;

  /// coefficient of effective porepressure
  const Real _P_coeff;

  /// coefficient of temperature
  const Real _T_coeff;

  /// coefficient of volumetric strain
  const Real _epv_coeff;

  /// whether epv_coeff has been set
  const bool _uses_volstrain;

  /// Strain (first const means we never want to dereference and change the value, second means we'll always be pointing to the same address after initialization (like a reference))
  const OptionalMaterialProperty<Real> & _vol_strain_qp;

  /// d(strain)/(dvar) (first const means we never want to dereference and change the value, second means we'll always be pointing to the same address after initialization (like a reference))
  const OptionalMaterialProperty<std::vector<RealGradient>> & _dvol_strain_qp_dvar;

  /// whether P_coeff has been set
  const bool _uses_pf;

  /// Effective porepressure at the quadpoints or nodes
  const OptionalMaterialProperty<Real> & _pf_nodal;
  const OptionalMaterialProperty<Real> & _pf_qp;
  const MaterialProperty<Real> * _pf;

  /// d(effective porepressure)/(d porflow variable)
  const OptionalMaterialProperty<std::vector<Real>> & _dpf_dvar_nodal;
  const OptionalMaterialProperty<std::vector<Real>> & _dpf_dvar_qp;
  const MaterialProperty<std::vector<Real>> * _dpf_dvar;

  /// whether T_coeff has been set
  const bool _uses_T;

  /// Temperature at the quadpoints or nodes
  const OptionalMaterialProperty<Real> & _temperature_nodal;
  const OptionalMaterialProperty<Real> & _temperature_qp;
  const MaterialProperty<Real> * _temperature;

  /// d(temperature)/(d porflow variable)
  const OptionalMaterialProperty<std::vector<Real>> & _dtemperature_dvar_nodal;
  const OptionalMaterialProperty<std::vector<Real>> & _dtemperature_dvar_qp;
  const MaterialProperty<std::vector<Real>> * _dtemperature_dvar;

  /// If the linear relationship produces porosity < _porosity_min, then porosity is set to _porosity_min.  This means the derivatives of it will be zero.  However, this gives poor NR convergence, so the derivatives are set to _zero_modifier * (values that are relevant for porosity_min) to hint to the NR that porosity is not always constant.
  const Real _zero_modifier;
};
