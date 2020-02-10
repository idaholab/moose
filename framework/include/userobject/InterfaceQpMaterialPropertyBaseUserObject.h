//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceQpUserObjectBase.h"

// Forward declarations
template <typename T = Real>
class InterfaceQpMaterialPropertyBaseUserObject;

template <>
InputParameters validParams<InterfaceQpMaterialPropertyBaseUserObject<>>();

/**
 * Specialization of InterfaceQpUserObjectBase for material properties. Material property type
 * specialization is achieved by specializing computeScalarMaterialProperty in derived classes.
 */
template <typename T>
class InterfaceQpMaterialPropertyBaseUserObject : public InterfaceQpUserObjectBase
{

public:
  static InputParameters validParams();
  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  InterfaceQpMaterialPropertyBaseUserObject(const InputParameters & parameters);
  virtual ~InterfaceQpMaterialPropertyBaseUserObject(){};

protected:
  /* method computing the real value*/
  virtual Real computeRealValue(const unsigned int qp) override final
  {
    if (!_compute_rate && !_compute_increment)
    {
      return computeInterfaceValueType(computeScalarMaterialProperty(&_prop, qp),
                                       computeScalarMaterialProperty(&_prop_neighbor, qp));
    }
    else
    {

      Real value_master = 0;
      Real value_slave = 0;
      if (_dt != 0)
      {

        value_master = (computeScalarMaterialProperty(&_prop, qp) -
                        computeScalarMaterialProperty(_prop_old, qp));
        value_slave = (computeScalarMaterialProperty(&_prop_neighbor, qp) -
                       computeScalarMaterialProperty(_prop_neighbor_old, qp));
        if (_compute_rate)
        {
          value_master /= _dt;
          value_slave /= _dt;
        }
      }
      return computeInterfaceValueType(value_master, value_slave);
    }
  };

  /*template for computing a scalar material property value*/
  virtual Real computeScalarMaterialProperty(const MaterialProperty<T> *,
                                             const unsigned int qp) = 0;

  /// the material property and neighbor material property current and old value
  ///@{
  const MaterialProperty<T> & _prop;
  const MaterialProperty<T> & _prop_neighbor;
  const MaterialProperty<T> * _prop_old;
  const MaterialProperty<T> * _prop_neighbor_old;
  ///@}
};

template <typename T>
InputParameters
InterfaceQpMaterialPropertyBaseUserObject<T>::validParams()
{
  InputParameters params = InterfaceQpUserObjectBase::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "The material property name");
  params.addParam<MaterialPropertyName>("property_neighbor",
                                        "The neighbor neighbor material property name");
  params.addClassDescription(
      "Computes the interface material property value or rate across an interface. The value or "
      "rate is computed according to the provided interface_value_type paramter.");
  return params;
}

template <typename T>
InterfaceQpMaterialPropertyBaseUserObject<T>::InterfaceQpMaterialPropertyBaseUserObject(
    const InputParameters & parameters)
  : InterfaceQpUserObjectBase(parameters),
    _prop(getMaterialProperty<T>("property")),
    _prop_neighbor(parameters.isParamSetByUser("property_neighbor")
                       ? getNeighborMaterialProperty<T>("property_neighbor")
                       : getNeighborMaterialProperty<T>("property")),
    _prop_old(_compute_rate || _compute_increment ? &getMaterialPropertyOld<T>("property")
                                                  : nullptr),
    _prop_neighbor_old(_compute_rate || _compute_increment
                           ? (parameters.isParamSetByUser("property_neighbor")
                                  ? &getNeighborMaterialPropertyOld<T>("property_neighbor")
                                  : &getNeighborMaterialPropertyOld<T>("property"))
                           : nullptr)

{
}
