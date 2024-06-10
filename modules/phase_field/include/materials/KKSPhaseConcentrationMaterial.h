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

class KKSPhaseConcentrationMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  KKSPhaseConcentrationMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void initialSetup() override;
  virtual void computeQpProperties() override;

  /// Global concentrations
  const std::vector<const VariableValue *> _prop_c;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Switching functions
  const MaterialProperty<Real> & _prop_h;

  ///@{ Phase concentrations
  const std::vector<MaterialPropertyName> _ci_names;
  std::vector<MaterialProperty<Real> *> _prop_ci;
  std::vector<const MaterialProperty<Real> *> _ci_old;
  const std::vector<Real> _ci_IC;
  ///@}

  ///@{ Free energies
  const MaterialName _Fa_name;
  const MaterialName _Fb_name;
  std::vector<const MaterialProperty<Real> *> _prop_Fi;
  std::vector<MaterialProperty<Real> *> _Fi_copy;
  ///@}

  ///@{ Derivative of free energies wrt phase concentrations \f$ \frac d{dc_i} F_i \f$
  std::vector<const MaterialProperty<Real> *> _dFidci;
  std::vector<MaterialProperty<Real> *> _dFidci_copy;
  ///@}

  ///@{ Second derivative of free energies wrt phase concentrations \f$ \frac {d^2}{dc_i db_i} F_i
  /// \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2Fidcidbi;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2Fadc1db1_copy;
  ///@}

  /// Coupled variables of free energies
  const std::vector<VariableName> _args_names;

  /// Number of coupled variables of free energies
  const unsigned int _n_args;

  ///@{ Derivative of free energies wrt coupled variables \f$ \frac d{dq} F_a \f$
  std::vector<const MaterialProperty<Real> *> _dFadarg;
  std::vector<MaterialProperty<Real> *> _dFadarg_copy;
  ///@}

  ///@{ Derivative of free energies wrt coupled variables \f$ \frac d{dq} F_b \f$
  std::vector<const MaterialProperty<Real> *> _dFbdarg;
  std::vector<MaterialProperty<Real> *> _dFbdarg_copy;
  ///@}

  ///@{ Second derivative of free energy Fa wrt phase concentration ca and a coupled variable \f$
  /// \frac{d^2}{dc_a dq} F_a \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2Fadcadarg;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2Fadcadarg_copy;
  ///@}

  /// Number of nested Newton iteration
  MaterialProperty<Real> & _iter;

  ///@{ Absolute and relative tolerance of nested Newton iteration
  const Real _abs_tol;
  const Real _rel_tol;
  ///@}

  /// Add damping functionality to nested Newton solve
  const bool _damped_newton;

  ///@{ Material property that defines the confidence bounds for the newton solve
  MaterialName _condition_name;
  MaterialBase * _condition;
  const MaterialProperty<Real> * _C;
  ///@}

  /// Instantiation of the NestedSolve class
  NestedSolve _nested_solve;

  ///@{ Free energy instantiation of the MaterialBase class
  MaterialBase * _Fa;
  MaterialBase * _Fb;
  ///@}
};
