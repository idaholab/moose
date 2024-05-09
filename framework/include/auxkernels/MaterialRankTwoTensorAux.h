//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialAuxBase.h"
#include "RankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

/**
 * MaterialRankTwoTensorAux is designed to take the data in the RankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */

template <typename T, bool is_ad>
class MaterialRankTwoTensorAuxTempl : public MaterialAuxBaseTempl<T, is_ad>
{
public:
  static InputParameters validParams();

  MaterialRankTwoTensorAuxTempl(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  ///@{ tensor indices
  const unsigned int _i;
  const unsigned int _j;
  ///@}
};

typedef MaterialRankTwoTensorAuxTempl<RankTwoTensor, false> MaterialRankTwoTensorAux;
typedef MaterialRankTwoTensorAuxTempl<RankTwoTensor, true> ADMaterialRankTwoTensorAux;
typedef MaterialRankTwoTensorAuxTempl<SymmetricRankFourTensor, false>
    MaterialSymmetricRankFourTensorAux;
typedef MaterialRankTwoTensorAuxTempl<SymmetricRankFourTensor, true>
    ADMaterialSymmetricRankFourTensorAux;
