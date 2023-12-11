//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"

class KKSPhaseConcentrationMultiPhaseDerivatives : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  KKSPhaseConcentrationMultiPhaseDerivatives(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Names of global concentrations
  const std::vector<VariableName> _c_names;

  /// Phase parameters
  const std::vector<VariableName> _eta_names;

  /// Number of phase parameters
  const unsigned int _num_j;

  ///@{ Phase concentrations
  std::vector<const MaterialProperty<Real> *> _prop_ci;
  std::vector<MaterialPropertyName> _ci_names;
  ///@}

  /// Derivative of phase concentrations wrt etaj \f$ \frac d{d{eta_j}} c_i \f$
  std::vector<std::vector<std::vector<MaterialProperty<Real> *>>> _dcidetaj;

  /// Derivative of phase concentrations wrt global concentrations \f$ \frac d{db} c_i \f$
  std::vector<std::vector<std::vector<MaterialProperty<Real> *>>> _dcidb;

  /// Free energy names
  std::vector<MaterialName> _Fj_names;

  /** Second derivative of phase concentrations wrt two phase concentrations \f$ \frac {d^2}{dc_i
  db_i} F_i \f$
  */
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2Fidcidbi;

  ///@{ Switching functions
  std::vector<MaterialPropertyName> _hj_names;
  std::vector<const MaterialProperty<Real> *> _prop_hj;
  ///@}

  /// Derivatives of switching functions
  std::vector<std::vector<const MaterialProperty<Real> *>> _dhjdetai;
};
