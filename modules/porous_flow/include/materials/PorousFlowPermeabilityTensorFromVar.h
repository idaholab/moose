//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityBase.h"

/**
 * Material designed to provide the permeability tensor which is calculated
 * from a tensor multiplied by a scalar:
 * k = k_ijk * k0
 * where k_ijk is a tensor providing the anisotropy, and k0 is a scalar
 * variable.
 */
template <bool is_ad>
class PorousFlowPermeabilityTensorFromVarTempl : public PorousFlowPermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityTensorFromVarTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Permeability components
  /// Note: these can only be constant (Real constant Monomial auxvariables) so no AD version
  const VariableValue & _perm;

  /// Tensor multiplier k_ijk
  const RealTensorValue _k_anisotropy;

  usingPorousFlowPermeabilityBaseMembers;
};

typedef PorousFlowPermeabilityTensorFromVarTempl<false> PorousFlowPermeabilityTensorFromVar;
typedef PorousFlowPermeabilityTensorFromVarTempl<true> ADPorousFlowPermeabilityTensorFromVar;
