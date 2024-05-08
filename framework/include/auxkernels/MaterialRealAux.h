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
 * Object for passing a scalar, REAL material property to an AuxVariable
 */
template <bool is_ad, bool is_functor>
class MaterialRealAuxTempl : public MaterialAuxBaseTempl<Real, is_ad, is_functor>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters Input parameters for this object
   */
  MaterialRealAuxTempl(const InputParameters & parameters);

protected:
  /// Returns the material property values at quadrature points
  Real getRealValue() override;
};

typedef MaterialRealAuxTempl<false, false> MaterialRealAux;
typedef MaterialRealAuxTempl<true, false> ADMaterialRealAux;
typedef MaterialRealAuxTempl<false, true> FunctorMaterialRealAux;
typedef MaterialRealAuxTempl<true, true> ADFunctorMaterialRealAux;
