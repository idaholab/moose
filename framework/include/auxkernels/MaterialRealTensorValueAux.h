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
 * AuxKernel for outputting a RealTensorValue material property component to an AuxVariable
 */
template <bool is_ad>
class MaterialRealTensorValueAuxTempl : public MaterialAuxBaseTempl<RealTensorValue, is_ad>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this AuxKernel
   */
  MaterialRealTensorValueAuxTempl(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// The row index to output
  unsigned int _row;

  /// The column index to output
  unsigned int _col;
};

typedef MaterialRealTensorValueAuxTempl<false> MaterialRealTensorValueAux;
typedef MaterialRealTensorValueAuxTempl<true> ADMaterialRealTensorValueAux;
