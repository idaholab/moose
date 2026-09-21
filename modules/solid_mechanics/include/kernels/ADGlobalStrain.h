//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "ADKernelScalarBase.h"
#include "RankTwoTensor.h"

#include <utility>
#include <vector>

class GlobalStrainPeriodicDirUserObject;

/**
 * Integrates the selected diagonal or off-diagonal stress components into the
 * residual of a global scalar strain variable. Instantiate this object once for
 * the diagonal variable and, when needed, once for the off-diagonal variable.
 */
class ADGlobalStrain : public ADKernelScalarBase
{
public:
  static InputParameters validParams();

  ADGlobalStrain(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;
  ADReal computeScalarQpResidual() override;

  void assignComponentIndices();

  const std::string _base_name;
  const ADMaterialProperty<RankTwoTensor> & _stress;

  const GlobalStrainPeriodicDirUserObject & _periodicity_uo;
  const VectorValue<bool> & _periodic_dir;

  /// True for xx/yy/zz; false for xy/xz/yz.
  const bool _use_diagonal_components;

  std::vector<std::pair<unsigned int, unsigned int>> _components;
  const unsigned int _dim;

  RankTwoTensor _applied_stress_tensor;
};
