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
#include "NestedSolve.h"

class KKSPhaseConcentrationDerivatives : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  KKSPhaseConcentrationDerivatives(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Names of global concentrations
  const std::vector<VariableName> _c_names;

  /// Phase parameter
  const VariableName _eta_name;

  ///@{ Phase concentrations
  const std::vector<MaterialPropertyName> _ci_names;
  std::vector<const MaterialProperty<Real> *> _prop_ci;
  ///@}

  /// Derivative of phase concentrations wrt global concentrations \f$ \frac d{db} c_i \f$
  std::vector<std::vector<std::vector<MaterialProperty<Real> *>>> _dcidb;

  /// Derivative of phase concentrations wrt eta \f$ \frac d{d{eta}} c_i \f$
  std::vector<std::vector<MaterialProperty<Real> *>> _dcideta;

  ///@{ Free energy names
  const MaterialName _Fa_name;
  const MaterialName _Fb_name;
  ///@}

  /** Second derivative of phase concentrations wrt two phase concentrations \f$ \frac {d^2}{dc_i
   db_i} F_i \f$
  */
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2Fidcidbi;

  /// Switching function
  const MaterialProperty<Real> & _prop_h;

  /// Derivative of switching function
  const MaterialProperty<Real> & _prop_dh;
};
