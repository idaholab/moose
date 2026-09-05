//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSCZMInterfaceKernelBase.h"

/// DG cohesive zone model kernel for the small strain formulation.
/// This kernel assummes the traction sepration law only depends from the
/// displacement jump. One kernel is required for each displacement component
class ADSCZMInterfaceKernelSmallStrain : public ADSCZMInterfaceKernelBase
{
public:
  static InputParameters validParams();
  ADSCZMInterfaceKernelSmallStrain(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// The stress tensor
  const ADMaterialProperty<RankTwoTensor> & _stress;
  /// The stress tensor from the neighbor side
  const ADMaterialProperty<RankTwoTensor> & _stress_neighbor;

  /// Whether to add the directional correction term
  const bool _directional_correction;
};
