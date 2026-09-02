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

/**
 * Enforces equality of the chemical potentials in two phases,
 *
 * \f$ \frac{1}{k_a}\frac{\partial F_a}{\partial c_a}
 *     = \frac{1}{k_b}\frac{\partial F_b}{\partial c_b}. \f$
 *
 * The nonlinear variable is \f$c_a\f$ and `cb` supplies \f$c_b\f$.
 * The template provides the shared AD/non-AD residual implementation. The
 * non-AD specialization below adds the explicitly calculated Jacobians.
 */
template <bool is_ad>
class KKSPhaseChemicalPotentialTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>
{
public:
  static InputParameters validParams();

  KKSPhaseChemicalPotentialTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual() override;
  void initialSetup() override;

  /// Name of the coupled phase-b concentration.
  const VariableName _cb_name;

  /// Actual free-energy material-property names supplied through the input parameters.
  const MaterialPropertyName _fa_name;
  const MaterialPropertyName _fb_name;

  /// First free-energy derivatives used by both the AD and non-AD residuals.
  const GenericMaterialProperty<Real, is_ad> & _dfadca;
  const GenericMaterialProperty<Real, is_ad> & _dfbdcb;

  /// Site fractions used by sublattice KKS; both default to one for ordinary KKS.
  const Real _ka;
  const Real _kb;

  usingGenericKernelMembers;
};

/**
 * Non-AD implementation. It retains the upstream hand-calculated diagonal and
 * off-diagonal Jacobians.
 */
class KKSPhaseChemicalPotential : public KKSPhaseChemicalPotentialTempl<false>
{
public:
  KKSPhaseChemicalPotential(const InputParameters & parameters);

protected:
  Real computeQpJacobian() override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const MaterialProperty<Real> & _d2fadca2;
  const MaterialProperty<Real> & _d2fbdcbca;

  std::vector<const MaterialProperty<Real> *> _d2fadcadarg;
  std::vector<const MaterialProperty<Real> *> _d2fbdcbdarg;
};

/// AD implementation. Its Jacobian is generated from the shared AD residual.
using ADKKSPhaseChemicalPotential = KKSPhaseChemicalPotentialTempl<true>;