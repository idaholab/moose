/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVEMULTIPHASEBASE_H
#define DERIVATIVEMULTIPHASEBASE_H

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations
class DerivativeMultiPhaseBase;

template <>
InputParameters validParams<DerivativeMultiPhaseBase>();

/**
 * DerivativeMaterial child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class DerivativeMultiPhaseBase : public DerivativeFunctionMaterialBase
{
public:
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

#endif // DERIVATIVEMULTIPHASEBASE_H
