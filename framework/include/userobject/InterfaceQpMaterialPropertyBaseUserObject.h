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
 * This userobject collect values of a variable across an interface for each QP and compute a
 * scalar. The computed scalar value depends on the given parameter _interface_value_type\
 * _interface_value_type (see IntervafeValueTools).
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
  virtual Real computeRealValueMaster(const unsigned int /*qp*/) override = 0;
  virtual Real computeRealValueSlave(const unsigned int /*qp*/) override = 0;
  const MaterialProperty<T> & _prop;
  const MaterialProperty<T> & _prop_neighbor;
  const MaterialProperty<T> * _prop_old;
  const MaterialProperty<T> * _prop_neighbor_old;
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
    _prop_old(_compute_rate ? &getMaterialPropertyOld<T>("property") : nullptr),
    _prop_neighbor_old(_compute_rate
                           ? (parameters.isParamSetByUser("property_neighbor")
                                  ? &getNeighborMaterialPropertyOld<T>("property_neighbor")
                                  : &getNeighborMaterialPropertyOld<T>("property"))
                           : nullptr)

{
}
