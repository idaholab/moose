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
 * Kernel = grad(test) * thermal_conductivity * grad(temperature)
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowHeatConductionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowHeatConductionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Thermal conductivity at the quadpoints
  const GenericMaterialProperty<RealTensorValue, is_ad> & _la;

  /// d(thermal conductivity at the quadpoints)/d(PorousFlow variable) -- null for AD
  const MaterialProperty<std::vector<RealTensorValue>> * const _dla_dvar;

  /// grad(temperature)
  const GenericMaterialProperty<RealGradient, is_ad> & _grad_t;

  /// d(gradT)/d(PorousFlow variable) -- null for AD
  const MaterialProperty<std::vector<RealGradient>> * const _dgrad_t_dvar;

  /// d(gradT)/d(grad PorousFlow variable) -- null for AD
  const MaterialProperty<std::vector<Real>> * const _dgrad_t_dgradvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowHeatConductionTempl<false> PorousFlowHeatConduction;
typedef PorousFlowHeatConductionTempl<true> ADPorousFlowHeatConduction;
