//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITY_H
#define POROUSFLOWPOROSITY_H

#include "PorousFlowPorosityExponentialBase.h"

// Forward Declarations
class PorousFlowPorosity;

template <>
InputParameters validParams<PorousFlowPorosity>();

/**
 * Material designed to provide the porosity in PorousFlow simulations
 * chemistry + biot + (phi0 - reference_chemistry - biot) * exp(-vol_strain
 *    + coeff * (effective_pressure - reference_pressure)
 *    + thermal_exp_coeff * (temperature - reference_temperature))
 */
class PorousFlowPorosity : public PorousFlowPorosityExponentialBase
{
public:
  PorousFlowPorosity(const InputParameters & parameters);

protected:
  virtual Real atNegInfinityQp() const override;
  virtual Real datNegInfinityQp(unsigned pvar) const override;
  virtual Real atZeroQp() const override;
  virtual Real datZeroQp(unsigned pvar) const override;
  virtual Real decayQp() const override;
  virtual Real ddecayQp_dvar(unsigned pvar) const override;
  virtual RealGradient ddecayQp_dgradvar(unsigned pvar) const override;

  /// porosity is a function of volumetric strain
  const bool _mechanical;

  /// porosity is a function of effective porepressure
  const bool _fluid;

  /// porosity is a function of temperature
  const bool _thermal;

  /// porosity is a function of chemistry
  const bool _chemical;

  /// porosity at zero strain and zero porepressure and zero temperature
  const VariableValue & _phi0;

  /// biot coefficient
  const Real _biot;

  /// thermal expansion coefficient of the solid porous skeleton
  const Real _exp_coeff;

  /// drained bulk modulus of the porous skeleton
  const Real _solid_bulk;

  /// short-hand number (biot-1)/solid_bulk
  const Real _coeff;

  /// reference temperature
  const VariableValue & _t_reference;

  /// reference porepressure
  const VariableValue & _p_reference;

  /// number of reference mineral concentrations provided by user
  const unsigned _num_c_ref;

  /// reference mineral concentrations
  std::vector<const VariableValue *> _c_reference;

  /// number of reference mineral concentrations provided by user
  const unsigned _num_initial_c;

  /// reference mineral concentrations
  std::vector<const VariableValue *> _initial_c;

  /// weights for the mineral concentrations
  std::vector<Real> _c_weights;

  const MaterialProperty<Real> & _porosity_old;

  /// strain (first const means we never want to dereference and change the value, second means we'll always be pointing to the same address after initialization (like a reference))
  const MaterialProperty<Real> * const _vol_strain_qp;

  /// d(strain)/(dvar) (first const means we never want to dereference and change the value, second means we'll always be pointing to the same address after initialization (like a reference))
  const MaterialProperty<std::vector<RealGradient>> * const _dvol_strain_qp_dvar;

  /// effective porepressure at the quadpoints or nodes
  const MaterialProperty<Real> * const _pf;

  /// d(effective porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> * const _dpf_dvar;

  /// temperature at the quadpoints or nodes
  const MaterialProperty<Real> * const _temperature;

  /// d(temperature)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;

  /// old value of mineral concentration at the quadpoints or nodes
  const MaterialProperty<std::vector<Real>> * const _mineral_conc_old;

  /// reaction rate of mineralisation
  const MaterialProperty<std::vector<Real>> * const _reaction_rate;

  /// d(reaction_rate_conc)/d(porflow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dreaction_rate_dvar;
};

#endif // POROUSFLOWPOROSITY_H
