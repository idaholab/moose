//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelValue.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * Enforce sum of phase concentrations to be the real concentration.
 *
 * \f$ c = h_1(\eta_1,\eta_2,\eta_3,...) c_1 + h_2(\eta_1,\eta_2,\eta_3,...) c_2
 * + h_3(\eta_1,\eta_2,\eta_3,..) c_3 + ... \f$
 *
 * The non-linear variable for this Kernel is one of  the concentrations \f$ c_i \f$, while
 * \f$ c_j \neq c_i \f$ and \f$ c \f$ are supplied as coupled variables.
 * The other phase concentrations are set as non-linear variables using multiple
 * KKSPhaseChemicalPotential kernels.
 *
 * \see KKSPhaseChemicalPotential
 */
class KKSMultiPhaseConcentration
  : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelValue>>
{
public:
  static InputParameters validParams();

  KKSMultiPhaseConcentration(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _num_j;
  const std::vector<const VariableValue *> _cj;
  const JvarMap & _cj_map;

  /// Position of the nonlinear variable in the list of cj's
  int _k;

  const VariableValue & _c;
  unsigned int _c_var;

  /// Switching functions for each phase \f$ h_j \f$
  std::vector<MaterialPropertyName> _hj_names;
  std::vector<const MaterialProperty<Real> *> _prop_hj;

  /// Order parameters for each phase \f$ \eta_j \f$
  std::vector<VariableName> _eta_names;
  const JvarMap & _eta_map;

  /// Derivative of the switching function \f$ \frac d{d\eta} h(\eta) \f$
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dhjdetai;
};
