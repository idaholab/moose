//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernelCurl.h"

template <bool is_ad>
class VectorFEWaveTempl : public GenericKernelCurl<is_ad>
{
public:
  static InputParameters validParams();

  VectorFEWaveTempl(const InputParameters & parameters);

protected:
virtual GenericReal<is_ad> computeQpResidual() override;
virtual Real computeQpJacobian() override;

  /// x component forcing function
  const Function & _x_ffn;
  /// y component forcing function
  const Function & _y_ffn;
  /// z component forcing function
  const Function & _z_ffn;

  usingGenericKernelCurlMembers;
};

typedef VectorFEWaveTempl<false> VectorFEWave;
typedef VectorFEWaveTempl<true> ADVectorFEWave;
