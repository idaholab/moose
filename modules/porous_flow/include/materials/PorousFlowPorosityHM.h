//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYHM_H
#define POROUSFLOWPOROSITYHM_H

#include "PorousFlowPorosityExponentialBase.h"

// Forward Declarations
class PorousFlowPorosityHM;

template <>
InputParameters validParams<PorousFlowPorosityHM>();

/**
 * Material designed to provide the porosity in hydro-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain +
 * (biot-1)*(effective_porepressure-reference_pressure)/solid_bulk)
 */
class PorousFlowPorosityHM : public PorousFlowPorosityExponentialBase
{
public:
  PorousFlowPorosityHM(const InputParameters & parameters);

protected:
  virtual Real atNegInfinityQp() const override;
  virtual Real atZeroQp() const override;
  virtual Real decayQp() const override;
  virtual Real ddecayQp_dvar(unsigned pvar) const override;
  virtual RealGradient ddecayQp_dgradvar(unsigned pvar) const override;

  /// porosity at zero strain and zero porepressure
  const VariableValue & _phi0;

  /// biot coefficient
  const Real _biot;

  /// drained bulk modulus of the porous skeleton
  const Real _solid_bulk;

  /// short-hand number (biot-1)/solid_bulk
  const Real _coeff;

  /// reference porepressure
  const VariableValue & _p_reference;

  /// number of displacement variables
  const unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// strain
  const MaterialProperty<Real> & _vol_strain_qp;

  /// d(strain)/(dvar)
  const MaterialProperty<std::vector<RealGradient>> & _dvol_strain_qp_dvar;

  /// effective porepressure
  const MaterialProperty<Real> & _pf;

  /// d(effective porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> & _dpf_dvar;
};

#endif // POROUSFLOWPOROSITYHM_H
