//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
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
template <bool is_ad>
class KKSMultiPhaseConcentrationTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>
{
public:
  static InputParameters validParams();

  KKSMultiPhaseConcentrationTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual() override;

  /// Number of phases/concentrations.
  const unsigned int _num_j;

  /// Phase concentrations c_j.
  const std::vector<const GenericVariableValue<is_ad> *> _cj;

  /// Position of this kernel's nonlinear variable in the c_j list.
  int _k;

  /// Physical concentration c.
  const GenericVariableValue<is_ad> & _c;

  /// Switching-function property names and values.
  const std::vector<MaterialPropertyName> _hj_names;
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _prop_hj;

  /// Order-parameter names, in the same order as the phases.
  std::vector<VariableName> _eta_names;

  usingGenericKernelMembers;
};

/**
 * Non-AD implementation with hand-calculated Jacobian entries.
 */
class KKSMultiPhaseConcentration : public KKSMultiPhaseConcentrationTempl<false>
{
public:
  KKSMultiPhaseConcentration(const InputParameters & parameters);

protected:
  Real computeQpJacobian() override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const JvarMap & _cj_map;
  const unsigned int _c_var;
  const JvarMap & _eta_map;

  /// dh_j/deta_i, indexed first by phase j and then order parameter i.
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dhjdetai;
};

/// AD implementation; its Jacobian is generated from the shared AD residual.
using ADKKSMultiPhaseConcentration = KKSMultiPhaseConcentrationTempl<true>;