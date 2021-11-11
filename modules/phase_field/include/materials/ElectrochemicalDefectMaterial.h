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

/**
 * This material calculates defect-specific parameters for the grand potential sintering model
 * with multiple defect species coupled with electrochemistry.
 */
class ElectrochemicalDefectMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ElectrochemicalDefectMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// number of solid phase order paramters
  const unsigned int _neta;

  /// solid phase order parameters
  std::vector<const VariableValue *> _eta;
  std::vector<VariableName> _eta_name;

  /// chemical potential
  const VariableValue & _w;
  const NonlinearVariableName _w_name;

  /// void phase order parameter
  const VariableValue & _phi;
  const NonlinearVariableName _phi_name;

  /// minimum in energy wrt vacancy concentration in solid
  const MaterialPropertyName _ns_min_name;
  const MaterialProperty<Real> & _ns_min;
  std::vector<const MaterialProperty<Real> *> _dns_min;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2ns_min;

  /// temperature
  const VariableValue & _temp;

  /// electric potential
  const VariableValue & _v;

  /// Gradient of electric potential
  const VariableGradient & _grad_V;

  /// void energy coefficient
  const MaterialProperty<Real> & _kv;
  /// solid energy coefficient
  const MaterialProperty<Real> & _ks;

  /// void phase switching function
  const MaterialProperty<Real> & _hv;
  const MaterialProperty<Real> & _dhv;
  const MaterialProperty<Real> & _d2hv;

  /// solid phase switching function
  const MaterialProperty<Real> & _hs;
  const MaterialProperty<Real> & _dhs;
  const MaterialProperty<Real> & _d2hs;

  /// susceptibility and derivatives
  MaterialPropertyName _chi_name;
  MaterialProperty<Real> & _chi;
  MaterialProperty<Real> & _dchidphi;
  MaterialProperty<Real> & _dchidw;
  MaterialProperty<Real> & _d2chidphi2;
  MaterialProperty<Real> & _d2chidw2;
  MaterialProperty<Real> & _d2chidphidw;

  /// void phase vacancy density
  MaterialPropertyName _nv_name;
  MaterialProperty<Real> & _nv;
  MaterialProperty<Real> & _dnvdw;

  /// solid phase vacancy density
  MaterialPropertyName _ns_name;
  MaterialProperty<Real> & _ns;
  MaterialProperty<Real> & _dnsdw;
  MaterialProperty<Real> & _d2nsdw2;
  std::vector<MaterialProperty<Real> *> _dns;
  std::vector<MaterialProperty<Real> *> _d2nsdwdeta;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2ns;

  /// Type of energy function to use for the solid phase
  const MooseEnum _solid_energy;

  /// Boltzmann constant
  const Real _kB;

  /// Electron charge
  const Real _e;

  /// Defect species charge
  const int _z;

  /// Voltage scale
  const Real _v_scale;

  /// Solid phase relative permittivity
  const Real _eps_r;

  /// minimum in energy wrt vacancy concentration in void
  const Real _nv_min;
};
