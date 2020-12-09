//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericRayKernel.h"

template <bool is_ad>
class CoupledLineSourceRayKernelTestTempl : public GenericRayKernel<is_ad>
{
public:
  CoupledLineSourceRayKernelTestTempl(const InputParameters & params);

  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual GenericReal<is_ad> computeQpOffDiagJacobian(const unsigned int jvar_num) override;

  const unsigned int _coupled;
  const GenericVariableValue<is_ad> & _coupled_val;

  usingGenericRayKernelMembers;
};

typedef CoupledLineSourceRayKernelTestTempl<false> CoupledLineSourceRayKernelTest;
typedef CoupledLineSourceRayKernelTestTempl<true> ADCoupledLineSourceRayKernelTest;
