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
 * This material calculates necessary parameters for the grand potential sintering model
 * with multiple defect species coupled with electrochemistry.
 */
class ElectrochemicalSinteringMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ElectrochemicalSinteringMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// number of solid phase order paramters
  const unsigned int _neta;

  /// solid phase order parameters
  std::vector<const VariableValue *> _eta;
  std::vector<VariableName> _eta_name;

  /// number of defect species
  const unsigned int _ndefects;

  /// chemical potentials for defects
  std::vector<const VariableValue *> _w;
  std::vector<VariableName> _w_name;

  /// void phase order parameter
  const VariableValue & _phi;
  const NonlinearVariableName _phi_name;

  /// minima in energy wrt vacancy concentration in solid
  std::vector<MaterialPropertyName> _ns_min_names;
  std::vector<const MaterialProperty<Real> *> _ns_min;
  std::vector<std::vector<const MaterialProperty<Real> *>> _dns_min;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2ns_min;

  /// temperature
  const VariableValue & _temp;

  /// electric potential
  const VariableValue & _v;

  /// Gradient of electric potential
  const VariableGradient & _grad_V;

  /// Names of void energy coefficient for each defect
  std::vector<MaterialPropertyName> _kv_names;
  /// void energy coefficients for each defect
  std::vector<const MaterialProperty<Real> *> _kv;
  /// Names of solid energy coefficient for each defect
  std::vector<MaterialPropertyName> _ks_names;
  /// solid energy coefficient for each defect
  std::vector<const MaterialProperty<Real> *> _ks;

  /// void phase switching function
  MaterialProperty<Real> & _hv;
  MaterialProperty<Real> & _dhv;
  MaterialProperty<Real> & _d2hv;

  /// solid phase switching function
  MaterialProperty<Real> & _hs;
  MaterialProperty<Real> & _dhs;
  MaterialProperty<Real> & _d2hs;

  /// void phase grand potential density
  MaterialProperty<Real> & _omegav;
  std::vector<MaterialProperty<Real> *> _domegavdw;
  std::vector<MaterialProperty<Real> *> _d2omegavdw2;

  /// solid phase grand potential density
  MaterialProperty<Real> & _omegas;
  std::vector<MaterialProperty<Real> *> _domegasdw;
  std::vector<MaterialProperty<Real> *> _d2omegasdw2;
  std::vector<MaterialProperty<Real> *> _domegasdeta;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2omegasdwdeta;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2omegasdetadeta;

  /// energy barrier coefficient
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _dmu;
  MaterialProperty<Real> & _d2mu;

  /// gradient energy coefficient
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _dkappa;
  MaterialProperty<Real> & _d2kappa;

  /// interface profile coefficient
  MaterialProperty<Real> & _gamma;

  /// surface energy
  const Real _sigma_s;

  /// grain boundary energy
  const Real _sigma_gb;

  /// interface width
  const Real _int_width;

  /// Parameter to determine accuracy of surface/GB phase switching function
  const Real _switch;

  /// Type of energy function to use for the solid phase
  const MooseEnum _solid_energy;

  /// mu value on surfaces
  const Real _mu_s;

  /// mu value on grain boundaries
  const Real _mu_gb;

  /// kappa value on surfaces
  const Real _kappa_s;

  /// kappa value on grain boundaries
  const Real _kappa_gb;

  /// Boltzmann constant
  const Real _kB;

  /// Electron charge
  const Real _e;

  /// Defects species charges in units of e
  const std::vector<int> _z;

  /// Voltage scale
  const Real _v_scale;

  /// Solid phase relative permittivity
  const Real _eps_r;

  /// minima in energy wrt vacancy concentration in void
  const std::vector<Real> _nv_min;
};
