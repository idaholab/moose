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

// Forward Declarations
template <bool>
class MaterialRealAuxTempl;
typedef MaterialRealAuxTempl<false> MaterialRealAux;

/**
 * Object for passing a scalar, REAL material property to an AuxVariable
 */
template <bool is_ad>
class MaterialRealAuxTempl : public MaterialAuxBaseTempl<Real, is_ad>
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

typedef MaterialRealAuxTempl<true> ADMaterialRealAux;
