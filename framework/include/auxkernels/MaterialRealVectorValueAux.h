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
#include "SymmetricRankTwoTensor.h"

/**
 * AuxKernel for outputting a RealVectorValue material property component to an AuxVariable
 */
template <typename T, bool is_ad, bool is_functor>
class MaterialRealVectorValueAuxTempl : public MaterialAuxBaseTempl<T, is_ad, is_functor>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialRealVectorValueAuxTempl(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// The vector component to output
  unsigned int _component;
};

typedef MaterialRealVectorValueAuxTempl<RealVectorValue, false, false> MaterialRealVectorValueAux;
typedef MaterialRealVectorValueAuxTempl<RealVectorValue, true, false> ADMaterialRealVectorValueAux;
typedef MaterialRealVectorValueAuxTempl<RealVectorValue, false, true>
    FunctorMaterialRealVectorValueAux;
typedef MaterialRealVectorValueAuxTempl<RealVectorValue, true, true>
    ADFunctorMaterialRealVectorValueAux;
typedef MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, false, false>
    MaterialSymmetricRankTwoTensorAux;
typedef MaterialRealVectorValueAuxTempl<SymmetricRankTwoTensor, true, false>
    ADMaterialSymmetricRankTwoTensorAux;
