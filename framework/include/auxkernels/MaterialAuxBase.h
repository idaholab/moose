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

// Forward declarations
template <typename T = Real>
class MaterialAuxBase;

template <>
InputParameters validParams<MaterialAuxBase<>>();

/**
 * A base class for the various Material related AuxKernal objects
 */
template <typename T>
class MaterialAuxBase : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialAuxBase(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Returns material property values at quadrature points
  virtual Real getRealValue() = 0;

  /// Reference to the material property for this AuxKernel
  const MaterialProperty<T> & _prop;

private:
  /// Multiplier for the material property
  const Real _factor;

  /// Value to be added to the material property
  const Real _offset;
};

template <typename T>
InputParameters
MaterialAuxBase<T>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "The scalar material property name");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>(
      "offset", 0, "The offset to add to your material property for visualization");
  return params;
}

template <typename T>
MaterialAuxBase<T>::MaterialAuxBase(const InputParameters & parameters)
  : AuxKernel(parameters),
    _prop(getMaterialProperty<T>("property")),
    _factor(getParam<Real>("factor")),
    _offset(getParam<Real>("offset"))
{
}

template <typename T>
Real
MaterialAuxBase<T>::computeValue()
{
  return _factor * getRealValue() + _offset;
}

