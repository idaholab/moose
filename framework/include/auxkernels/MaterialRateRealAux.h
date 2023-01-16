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
#include "MaterialRateAuxBase.h"

/**
 * Object for passing a scalar, REAL material property to an AuxVariable
 */
template <bool is_ad>
class MaterialRateRealAuxTempl : public MaterialRateAuxBaseTempl<Real, is_ad>
{
public:
  static InputParameters validParams();
  MaterialRateRealAuxTempl(const InputParameters & parameters);

protected:
  /// Returns the material rate property values at quadrature points
  Real getRealValue() override;
};
typedef MaterialRateRealAuxTempl<false> MaterialRateRealAux;
typedef MaterialRateRealAuxTempl<true> ADMaterialRateRealAux;
