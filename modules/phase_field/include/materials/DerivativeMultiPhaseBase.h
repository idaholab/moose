//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations

/**
 * DerivativeMaterial child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 */
class DerivativeMultiPhaseBase : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  DerivativeMultiPhaseBase(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeF();

  /**
   * If the variable a non-conserved OP this array holds the index into
   * the etas parameter vector for a given arg index, otherwise it holds -1
   */
  std::vector<int> _eta_index;

  /// Phase parameter (0=A-phase, 1=B-phase)
  std::vector<VariableValue *> _etas;

  /// name of the order parameter variable
  unsigned int _num_etas;
  std::vector<VariableName> _eta_names;
  std::vector<unsigned int> _eta_vars;

  /// phase derivative material names
  std::vector<MaterialPropertyName> _fi_names;
  unsigned int _num_fi;

  /// Function value of the i phase.
  std::vector<const MaterialProperty<Real> *> _prop_Fi;

  /// Derivatives of Fi w.r.t. arg[i]
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dFi;

  /// Second derivatives of Fi.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _prop_d2Fi;

  /// Third derivatives of Fi.
  std::vector<std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>>> _prop_d3Fi;

  /// phase switching function names
  std::vector<MaterialPropertyName> _hi_names;
  unsigned int _num_hi;

  /// Switching functions
  std::vector<const MaterialProperty<Real> *> _hi;

  /// Barrier function name
  MaterialPropertyName _g_name;

  /// Barrier function \f$ g(\eta_0, \eta_1, \dots, \eta_{n-1}) \f$
  const MaterialProperty<Real> & _g;

  /// Barrier function derivatives.
  std::vector<const MaterialProperty<Real> *> _dg;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2g;
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _d3g;

  /// Phase transformation energy barrier
  Real _W;
};
