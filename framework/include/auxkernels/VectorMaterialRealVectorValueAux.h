//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MaterialAuxBase.h"

/**
 * AuxKernel for outputting a RealVectorValue material property component to a vector AuxVariable
 */
template <bool is_ad>
class VectorMaterialRealVectorValueAuxTempl
  : public MaterialAuxBaseTempl<RealVectorValue, is_ad, RealVectorValue>
{
public:
  static InputParameters validParams();

  VectorMaterialRealVectorValueAuxTempl(const InputParameters & parameters);

protected:
  virtual RealVectorValue getRealValue() override;
};

typedef VectorMaterialRealVectorValueAuxTempl<false> VectorMaterialRealVectorValueAux;
typedef VectorMaterialRealVectorValueAuxTempl<true> ADVectorMaterialRealVectorValueAux;
