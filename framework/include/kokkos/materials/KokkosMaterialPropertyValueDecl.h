//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

#define usingKokkosMaterialPropertyValueBaseMembers(T, dimension)                                  \
  using MaterialPropertyValueBase<T, dimension>::_idx;                                             \
  using MaterialPropertyValueBase<T, dimension>::_data;                                            \
  using MaterialPropertyValueBase<T, dimension>::_value

namespace Moose::Kokkos
{

template <typename T, unsigned int dimension>
class MaterialProperty;

class Datum;

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
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION MaterialPropertyValueBase(const MaterialProperty<T, dimension> & property,
                                            const Datum & datum,
                                            const unsigned int qp);

  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION dof_id_type n(unsigned int dim) const { return _data->n(dim); }

protected:
  /**
   * Index into the property data storage
   */
  const dof_id_type _idx;
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
class MaterialPropertyValue : public MaterialPropertyValueBase<T, dimension>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, dimension);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION MaterialPropertyValue(const MaterialProperty<T, dimension> & property,
                                        const Datum & datum,
                                        const unsigned int qp);

  /**
   * Get the writeable reference of a property value
   * @param i The index of each dimension
   * @returns The writeable reference of the property value
   */
  template <typename... index_type>
  KOKKOS_FUNCTION T & operator()(index_type... i)
  {
    static_assert(sizeof...(i) == dimension,
                  "Number of arguments should match material property dimension");

    return (*_data)(i..., _idx);
  }
  /**
   * Get the const reference of a property value
   * @param i The index of each dimension
   * @returns The const reference of the property value
   */
  template <typename... index_type>
  KOKKOS_FUNCTION const T & operator()(index_type... i) const
  {
    static_assert(sizeof...(i) == dimension,
                  "Number of arguments should match material property dimension");

    return _data ? (*_data)(i..., _idx) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 0> : public MaterialPropertyValueBase<T, 0>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 0);

public:
  /**
   * Constructor
   * @param property The material property constructing this object
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 0> & property,
                        const Datum & datum,
                        unsigned int qp);

  /**
   * Get the const reference of a property value
   * @returns The const reference of the property value
   */
  KOKKOS_FUNCTION operator const T &() const { return _data ? (*_data)(_idx) : _value; }
  /**
   * Assign a value to the underlying property
   * @param value The value to assign
   */
  KOKKOS_FUNCTION auto & operator=(const T & value);
  /**
   * Copy a value from another property
   * @param value The property to copy
   */
  KOKKOS_FUNCTION auto & operator=(const MaterialPropertyValue<T, 0> & value);
};
///@}

} // namespace Moose::Kokkos
