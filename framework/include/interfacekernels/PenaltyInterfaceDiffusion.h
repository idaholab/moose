//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericInterfaceKernel.h"

/**
 * Interface kernel for interfacing diffusion between two variables on adjacent blocks
 */
template <typename T, bool is_ad>
class PenaltyInterfaceDiffusionTempl : public GenericInterfaceKernelTempl<T, is_ad>
{
public:
  static InputParameters validParams();

  PenaltyInterfaceDiffusionTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual(Moose::DGResidualType type) override;
  Real computeQpJacobian(Moose::DGJacobianType type) override;

  const Real _penalty;

  std::string _jump_prop_name;
  const GenericMaterialProperty<T, is_ad> * const _jump;

  usingGenericInterfaceKernelTemplMembers(T);
};

typedef PenaltyInterfaceDiffusionTempl<Real, false> PenaltyInterfaceDiffusion;
typedef PenaltyInterfaceDiffusionTempl<Real, true> ADPenaltyInterfaceDiffusion;
typedef PenaltyInterfaceDiffusionTempl<RealVectorValue, false> VectorPenaltyInterfaceDiffusion;
typedef PenaltyInterfaceDiffusionTempl<RealVectorValue, true> ADVectorPenaltyInterfaceDiffusion;
