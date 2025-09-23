//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterialProperty.h"
#include "KokkosDatum.h"

#define usingKokkosMaterialPropertyValueBaseMembers(T, dimension)                                  \
  using MaterialPropertyValueBase<T, dimension>::_qp;                                              \
  using MaterialPropertyValueBase<T, dimension>::_data;                                            \
  using MaterialPropertyValueBase<T, dimension>::_value

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos wrapper class for accessing the material property values of a single quadrature
 * point. The instances of this class are expected to be only created by the Kokkos material
 * properties as temporary objects.
 */
///@{
template <typename T, unsigned int dimension>
class MaterialPropertyValueBase
{
public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION MaterialPropertyValueBase(const MaterialProperty<T, dimension> & property,
                                            SubdomainID sid,
                                            dof_id_type qp)
    : _qp(qp), _data(property._default ? nullptr : &property._data[sid]), _value(property._value)
  {
  }

  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION dof_id_type n(unsigned int dim) const { return _data->n(dim); }

protected:
  /**
   * Global quadrature point index
   */
  const dof_id_type _qp;
  /**
   * Pointer to the property data storage
   */
  Array<T, dimension + 1> const * _data;
  /**
   * Default value
   */
  const T & _value;
};

template <typename T, unsigned int dimension>
class MaterialPropertyValue;

template <typename T>
class MaterialPropertyValue<T, 0> : public MaterialPropertyValueBase<T, 0>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 0);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 0> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 0>(property, sid, qp)
  {
  }
  /**
   * Get the const reference of a property value
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION operator const T &() const { return _data ? (*_data)(_qp) : _value; }
  /**
   * Assign a value to the underlying property
   * @param value The value to assign
   */
  KOKKOS_FUNCTION auto & operator=(const T & value)
  {
    (*_data)(_qp) = value;

    return *this;
  }
  /**
   * Copy a value from another property
   * @param value The property to copy
   */
  KOKKOS_FUNCTION auto & operator=(const MaterialPropertyValue<T, 0> & value)
  {
    (*_data)(_qp) = static_cast<T>(value);

    return *this;
  }
};

template <typename T>
class MaterialPropertyValue<T, 1> : public MaterialPropertyValueBase<T, 1>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 1);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 1> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 1>(property, sid, qp)
  {
  }
  /**
   * Get the writeable reference of a property value
   * @param i0 The first dimension index
   * @returns The writeable reference of the property value
   */
  KOKKOS_FUNCTION T & operator()(unsigned int i0) { return (*_data)(i0, _qp); }
  /**
   * Get the const reference of a property value
   * @param i0 The first dimension index
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION const T & operator()(unsigned int i0) const
  {
    return _data ? (*_data)(i0, _qp) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 2> : public MaterialPropertyValueBase<T, 2>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 2);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 2> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 2>(property, sid, qp)
  {
  }
  /**
   * Get the writeable reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @returns The writeable reference of the property value
   */
  KOKKOS_FUNCTION T & operator()(unsigned int i0, unsigned int i1) { return (*_data)(i0, i1, _qp); }
  /**
   * Get the const reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION const T & operator()(unsigned int i0, unsigned int i1) const
  {
    return _data ? (*_data)(i0, i1, _qp) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 3> : public MaterialPropertyValueBase<T, 3>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 3);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 3> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 3>(property, sid, qp)
  {
  }
  /**
   * Get the writeable reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @returns The writeable reference of the property value
   */
  KOKKOS_FUNCTION T & operator()(unsigned int i0, unsigned int i1, unsigned int i2)
  {
    return (*_data)(i0, i1, i2, _qp);
  }
  /**
   * Get the const reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION const T & operator()(unsigned int i0, unsigned int i1, unsigned int i2) const
  {
    return _data ? (*_data)(i0, i1, i2, _qp) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 4> : public MaterialPropertyValueBase<T, 4>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 4);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param sid The subdomain ID
   * @param qp The global quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 4> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 4>(property, sid, qp)
  {
  }
  /**
   * Get the writeable reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @param i3 The fourth dimension index
   * @returns The writeable reference of the property value
   */
  KOKKOS_FUNCTION T & operator()(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3)
  {
    return (*_data)(i0, i1, i2, i3, _qp);
  }
  /**
   * Get the const reference of a property value
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @param i3 The fourth dimension index
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION const T &
  operator()(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3) const
  {
    return _data ? (*_data)(i0, i1, i2, i3, _qp) : _value;
  }
};
///@}

template <typename T, unsigned int dimension>
KOKKOS_FUNCTION MaterialPropertyValue<T, dimension>
MaterialProperty<T, dimension>::operator()(Datum & datum, unsigned int qp) const
{
  auto & elem = datum.elem();
  auto qp_offset = datum.qpOffset();

  return MaterialPropertyValue<T, dimension>(*this, elem.subdomain, qp_offset + qp);
}

} // namespace Kokkos
} // namespace Moose
