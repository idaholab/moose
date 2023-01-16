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
 * A base class for the various Material related AuxKernal objects.
 * \p RT is short for return type
 */
template <typename T, bool is_ad, typename RT = Real>
class MaterialRateAuxBaseTempl : public MaterialAuxBaseTempl<RT, is_ad>
{
public:
  static InputParameters validParams();
  MaterialRateAuxBaseTempl(const InputParameters & parameters);

protected:
  /// Returns material rate property values at quadrature points
  virtual RT getRealValue() = 0;

  /// Reference to the old material property for this AuxKernel
  const MaterialProperty<T> & _prop_old;
};

template <typename T, bool is_ad, typename RT>
InputParameters
MaterialRateAuxBaseTempl<T, is_ad, RT>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RT, is_ad>::validParams();
  return params;
}

template <typename T, bool is_ad, typename RT>
MaterialRateAuxBaseTempl<T, is_ad, RT>::MaterialRateAuxBaseTempl(const InputParameters & parameters)
  : MaterialAuxBaseTempl<RT, is_ad>(parameters),
    _prop_old(this->template getMaterialPropertyOld<T>("property"))
{
}

template <typename T = Real>
using MaterialRateAuxBase = MaterialRateAuxBaseTempl<T, false, Real>;
