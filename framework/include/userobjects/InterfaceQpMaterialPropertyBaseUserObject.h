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
  /**
   * method defining the scalar value computation given a scalar material property value
   **/
  virtual Real computeRealValue(const unsigned int qp) override final;
  /**
   * method returning a scalar material property value given a generic T type
   **/
  virtual Real computeScalarMaterialProperty(const MaterialProperty<T> *,
                                             const unsigned int qp) = 0;

  /// the material property and neighbor material property current and old value
  ///@{
  const MaterialProperty<T> & _prop;
  const MaterialProperty<T> & _prop_neighbor;
  const MaterialProperty<T> * const _prop_old;
  const MaterialProperty<T> * const _prop_neighbor_old;
  ///@}
};

template <typename T>
InputParameters
InterfaceQpMaterialPropertyBaseUserObject<T>::validParams()
{
  InputParameters params = InterfaceQpUserObjectBase::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "The material property name");
  params.addParam<MaterialPropertyName>("property_neighbor", "The neighbor material property name");
  params.addClassDescription(
      "Computes the interface material property value or rate across an interface. The value or "
      "rate is computed according to the provided interface_value_type parameter.");
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
    _prop_old(_value_type > 0 ? &getMaterialPropertyOld<T>("property") : nullptr),
    _prop_neighbor_old(_value_type > 0
                           ? ((parameters.isParamSetByUser("property_neighbor")
                                   ? &getNeighborMaterialPropertyOld<T>("property_neighbor")
                                   : &getNeighborMaterialPropertyOld<T>("property")))
                           : nullptr)
{
}

template <typename T>
Real
InterfaceQpMaterialPropertyBaseUserObject<T>::computeRealValue(const unsigned int qp)
{
  Real value_primary = 0;
  Real value_secondary = 0;
  // using an if else here because a switch produce an unkown error in the docuemantion test
  if (_value_type == 0) /*value*/
  {
    value_primary = computeScalarMaterialProperty(&_prop, qp);
    value_secondary = computeScalarMaterialProperty(&_prop_neighbor, qp);
  }
  else if (_value_type == 1) /*rate*/
  {
    if (_dt != 0)
    {
      value_primary = (computeScalarMaterialProperty(&_prop, qp) -
                       computeScalarMaterialProperty(_prop_old, qp)) /
                      _dt;
      value_secondary = (computeScalarMaterialProperty(&_prop_neighbor, qp) -
                         computeScalarMaterialProperty(_prop_neighbor_old, qp)) /
                        _dt;
    }
  }
  else if (_value_type == 2) /*increment*/
  {
    value_primary =
        (computeScalarMaterialProperty(&_prop, qp) - computeScalarMaterialProperty(_prop_old, qp));
    value_secondary = (computeScalarMaterialProperty(&_prop_neighbor, qp) -
                       computeScalarMaterialProperty(_prop_neighbor_old, qp));
  }
  else
    mooseError("InterfaceQpMaterialPropertyBaseUserObject::computeRealValue the supplied "
               "value type has not been implemented");

  return computeInterfaceValueType(value_primary, value_secondary);
}
