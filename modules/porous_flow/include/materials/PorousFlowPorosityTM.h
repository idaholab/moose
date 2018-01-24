//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYTM_H
#define POROUSFLOWPOROSITYTM_H

#include "PorousFlowPorosityExponentialBase.h"

// Forward Declarations
class PorousFlowPorosityTM;

template <>
InputParameters validParams<PorousFlowPorosityTM>();

/**
 * Material designed to provide the porosity in thermo-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain + thermal_exp_coeff * (temperature - reference_temperature))
 */
class PorousFlowPorosityTM : public PorousFlowPorosityExponentialBase
{
public:
  PorousFlowPorosityTM(const InputParameters & parameters);

protected:
  virtual Real atNegInfinityQp() const override;
  virtual Real atZeroQp() const override;
  virtual Real decayQp() const override;
  virtual Real ddecayQp_dvar(unsigned pvar) const override;
  virtual RealGradient ddecayQp_dgradvar(unsigned pvar) const override;

  /// porosity at zero strain and zero temperature
  const VariableValue & _phi0;

  /// thermal expansion coefficient of the solid porous skeleton
  const Real _exp_coeff;

  /// reference temperature
  const VariableValue & _t_reference;

  /// number of displacement variables
  const unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// strain
  const MaterialProperty<Real> & _vol_strain_qp;

  /// d(strain)/(dvar)
  const MaterialProperty<std::vector<RealGradient>> & _dvol_strain_qp_dvar;

  /// temperature at quadpoints or nodes
  const MaterialProperty<Real> & _temperature;

  /// d(temperature)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> & _dtemperature_dvar;
};

#endif // POROUSFLOWPOROSITYTM_H
