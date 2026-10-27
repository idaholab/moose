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
 * PorousFlowEffectiveStressCoupling computes
 * -coefficient*effective_porepressure*grad_component(test)
 * where component is the spatial component (not a fluid component!)
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowEffectiveStressCouplingTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowEffectiveStressCouplingTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// The PorousFlow dictator that holds global info about the simulation
  const PorousFlowDictator & _dictator;

  /// Biot coefficient
  const Real _coefficient;

  /// The spatial component
  const unsigned int _component;

  /// Effective porepressure
  const GenericMaterialProperty<Real, is_ad> & _pf;

  /// d(effective porepressure)/(d porflow variable) -- null for AD
  const MaterialProperty<std::vector<Real>> * const _dpf_dvar;

  /// Whether an RZ coordinate system is being used
  const bool _rz;

  usingGenericKernelMembers;
};

typedef PorousFlowEffectiveStressCouplingTempl<false> PorousFlowEffectiveStressCoupling;
typedef PorousFlowEffectiveStressCouplingTempl<true> ADPorousFlowEffectiveStressCoupling;
