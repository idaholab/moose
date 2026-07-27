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
#include "PorousFlowDictator.h"

/**
 * Kernel = (desorped_mass - desorped_mass_old)/dt
 * It is NOT lumped to the nodes
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowDesorpedMassTimeDerivativeTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowDesorpedMassTimeDerivativeTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// The MOOSE variable number of the concentration variable
  const unsigned int _conc_var_number;

  /// The concentration variable (AD or non-AD)
  const GenericVariableValue<is_ad> & _conc;

  /// Old value of the concentration variable (always non-AD)
  const VariableValue & _conc_old;

  /// Porosity at the qps (AD or non-AD)
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// Old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// d(porosity)/d(PorousFlow variable) -- null for AD
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable) -- null for AD
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /**
   * Derivative of residual with respect to variable number jvar (non-AD path only)
   */
  Real computeQpJac(unsigned int jvar);

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowDesorpedMassTimeDerivativeTempl<false> PorousFlowDesorpedMassTimeDerivative;
typedef PorousFlowDesorpedMassTimeDerivativeTempl<true> ADPorousFlowDesorpedMassTimeDerivative;
