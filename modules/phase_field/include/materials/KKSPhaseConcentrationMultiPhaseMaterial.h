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

class KKSPhaseConcentrationMultiPhaseMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  KKSPhaseConcentrationMultiPhaseMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void initialSetup() override;
  virtual void computeQpProperties() override;

  /// Global concentrations
  const std::vector<const VariableValue *> _prop_c;

  /// Number of global concentrations
  const unsigned int _num_c;

  /// Number of phase parameters
  const unsigned int _num_j;

  ///@{Switching functions
  std::vector<MaterialPropertyName> _hj_names;
  std::vector<const MaterialProperty<Real> *> _prop_hj;
  ///@}

  ///@{Free energies
  const std::vector<MaterialName> _Fj_names;
  std::vector<const MaterialProperty<Real> *> _prop_Fi;
  ///@}

  ///@{Phase concentrations
  std::vector<MaterialPropertyName> _ci_names;
  std::vector<MaterialProperty<Real> *> _prop_ci;
  std::vector<const MaterialProperty<Real> *> _ci_old;
  std::vector<Real> _ci_IC;
  ///@}

  ///@{Derivative of free energies wrt phase concentrations \f$ \frac d{dc_i} F_i \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dFidci;
  ///@}

  ///@{Second derivative of free energies wrt phase concentrations \f$ \frac {d^2}{dc_i db_i} F_i
  /// \f$
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d2Fidcidbi;
  ///@}

  /// Coupled variables of free energies
  const std::vector<VariableName> _args_names;

  /// Number of coupled variables of free energies
  const unsigned int _n_args;

  ///@{Derivative of free energies wrt coupled variables \f$ \frac d{dq} F_i \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _dFidarg;
  ///@}

  ///@{Second derivative of free energy Fa wrt phase concentration ca and a coupled variable \f$
  /// \frac{d^2}{dc_a dq} F_a \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2F1dc1darg;
  ///@}

  /// Number of nested Newton iteration
  MaterialProperty<Real> & _iter;

  ///@{Absolute and relative tolerance of nested Newton iteration
  const Real _abs_tol;
  const Real _rel_tol;
  ///@}

  /// Check whether to use damping
  const bool _damped_newton;

  ///@{Condition that must be violated for damping to occur
  MaterialName _condition_name;
  MaterialBase * _condition;
  const MaterialProperty<Real> * _C;
  ///@}

  /// Instantiation of the NestedSolve class
  NestedSolve _nested_solve;

  /// Free energy instantiation of the MaterialBase class
  std::vector<MaterialBase *> _Fj_mat;
};
