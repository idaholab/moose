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
#include "AuxKernel.h"

/**
 * A base class for the various Material related AuxKernal objects.
 * \p RT is short for return type
 */
template <typename T, bool is_ad, typename RT = Real>
class MaterialAuxBaseTempl : public AuxKernelTempl<RT>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialAuxBaseTempl(const InputParameters & parameters);

protected:
  virtual RT computeValue() override;

  /// Returns material property values at quadrature points
  virtual RT getRealValue() = 0;

  /// Reference to the material property for this AuxKernel
  const GenericMaterialProperty<T, is_ad> & _prop;

private:
  /// Multiplier for the material property
  const Real _factor;

  /// Value to be added to the material property
  const RT _offset;
};

template <typename T, bool is_ad, typename RT>
InputParameters
MaterialAuxBaseTempl<T, is_ad, RT>::validParams()
{
  InputParameters params = AuxKernelTempl<RT>::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "The scalar material property name");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<RT>("offset", 0, "The offset to add to your material property for visualization");
  return params;
}

template <typename T, bool is_ad, typename RT>
MaterialAuxBaseTempl<T, is_ad, RT>::MaterialAuxBaseTempl(const InputParameters & parameters)
  : AuxKernelTempl<RT>(parameters),
    _prop(this->template getGenericMaterialProperty<T, is_ad>("property")),
    _factor(this->template getParam<Real>("factor")),
    _offset(this->template getParam<RT>("offset"))
{
}

template <typename T, bool is_ad, typename RT>
RT
MaterialAuxBaseTempl<T, is_ad, RT>::computeValue()
{
  return _factor * getRealValue() + _offset;
}

template <typename T = Real>
using MaterialAuxBase = MaterialAuxBaseTempl<T, false, Real>;
