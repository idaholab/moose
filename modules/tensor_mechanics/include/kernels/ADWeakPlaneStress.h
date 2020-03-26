//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * ADWeakPlaneStress is the automatic differentiation version of WeakPlaneStress
 */
class ADWeakPlaneStress : public ADKernelValue
{
public:
  static InputParameters validParams();

  ADWeakPlaneStress(const InputParameters & parameters);

protected:
  ADReal precomputeQpResidual() override;

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// The stress tensor that provides the out-of-plane stress
  const ADMaterialProperty<RankTwoTensor> & _stress;

  /// The direction of the out-of-plane strain variable
  const unsigned int _direction;
};
