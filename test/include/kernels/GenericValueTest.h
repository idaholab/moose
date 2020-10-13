//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

template <bool is_ad>
class GenericValueTestTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  GenericValueTestTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  usingGenericKernelMembers;
};

typedef GenericValueTestTempl<false> GenericValueTest;
typedef GenericValueTestTempl<true> ADGenericValueTest;
