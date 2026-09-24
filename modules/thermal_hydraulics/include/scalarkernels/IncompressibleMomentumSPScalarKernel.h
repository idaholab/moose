//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IncompressibleMomentumSPBase.h"

template <bool is_ad>
class IncompressibleMomentumSPScalarKernelTempl : public IncompressibleMomentumSPBaseTempl<is_ad>
{
  using Base = IncompressibleMomentumSPBaseTempl<is_ad>;

public:
  IncompressibleMomentumSPScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  /// Coupled reference pressure drop from inlet to outlet of the path
  const VariableValue & _dPc;
};

typedef IncompressibleMomentumSPScalarKernelTempl<false> IncompressibleMomentumSPScalarKernel;
typedef IncompressibleMomentumSPScalarKernelTempl<true> ADIncompressibleMomentumSPScalarKernel;
