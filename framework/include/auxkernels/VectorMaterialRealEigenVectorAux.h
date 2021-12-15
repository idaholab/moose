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

template <bool>
class VectorMaterialRealEigenVectorAuxTempl;
typedef VectorMaterialRealEigenVectorAuxTempl<false> VectorMaterialRealEigenVectorAux;

/**
 * AuxKernel for outputting a RealEigenVector material property
 * to an array AuxVariable
 */
template <bool is_ad>
class VectorMaterialRealEigenVectorAuxTempl
  : public MaterialAuxBaseTempl<RealEigenVector, is_ad, RealEigenVector>
{
public:
  VectorMaterialRealEigenVectorAuxTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  RealEigenVector getRealValue() override final;
};
