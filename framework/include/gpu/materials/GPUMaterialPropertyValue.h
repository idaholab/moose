//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUMaterialProperty.h"
#include "GPUDatum.h"

#define usingGPUMaterialPropertyValueBaseMembers(T, dimension)                                     \
  using GPUMaterialPropertyValueBase<T, dimension>::_qp;                                           \
  using GPUMaterialPropertyValueBase<T, dimension>::_data;                                         \
  using GPUMaterialPropertyValueBase<T, dimension>::_value;

template <typename T, unsigned int dimension>
class GPUMaterialPropertyValueBase
{
protected:
  // Current quadrature point
  const dof_id_type _qp;
  // Data array
  const GPUArray<T, dimension + 1> * _data;
  // Default value
  const T & _value;

public:
  KOKKOS_FUNCTION GPUMaterialPropertyValueBase(const GPUMaterialProperty<T, dimension> & property,
                                               SubdomainID sid,
                                               dof_id_type qp)
    : _qp(qp), _data(property._default ? nullptr : &property._data[sid]), _value(property._value)
  {
  }
  // Get the size of each dimension
  KOKKOS_FUNCTION uint64_t n(unsigned int dim) { return _data->n(dim); }
};

template <typename T, unsigned int dimension>
class GPUMaterialPropertyValue
{
};

template <typename T>
class GPUMaterialPropertyValue<T, 0> : public GPUMaterialPropertyValueBase<T, 0>
{
  usingGPUMaterialPropertyValueBaseMembers(T, 0);

public:
  KOKKOS_FUNCTION
  GPUMaterialPropertyValue(const GPUMaterialProperty<T, 0> & property,
                           SubdomainID sid,
                           dof_id_type qp)
    : GPUMaterialPropertyValueBase<T, 0>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION operator const T &() const { return _data ? (*_data)(_qp) : _value; }
  KOKKOS_FUNCTION auto & operator=(const T & data)
  {
    (*_data)(_qp) = data;

    return *this;
  }
  KOKKOS_FUNCTION auto & operator=(const GPUMaterialPropertyValue<T, 0> & property)
  {
    (*_data)(_qp) = static_cast<T>(property);

    return *this;
  }
};

template <typename T>
class GPUMaterialPropertyValue<T, 1> : public GPUMaterialPropertyValueBase<T, 1>
{
  usingGPUMaterialPropertyValueBaseMembers(T, 1);

public:
  KOKKOS_FUNCTION
  GPUMaterialPropertyValue(const GPUMaterialProperty<T, 1> & property,
                           SubdomainID sid,
                           dof_id_type qp)
    : GPUMaterialPropertyValueBase<T, 1>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x) { return (*_data)(x, _qp); }
  KOKKOS_FUNCTION const T & operator()(unsigned int x) const
  {
    return _data ? (*_data)(x, _qp) : _value;
  }
};

template <typename T>
class GPUMaterialPropertyValue<T, 2> : public GPUMaterialPropertyValueBase<T, 2>
{
  usingGPUMaterialPropertyValueBaseMembers(T, 2);

public:
  KOKKOS_FUNCTION
  GPUMaterialPropertyValue(const GPUMaterialProperty<T, 2> & property,
                           SubdomainID sid,
                           dof_id_type qp)
    : GPUMaterialPropertyValueBase<T, 2>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x, unsigned int y) { return (*_data)(x, y, _qp); }
  KOKKOS_FUNCTION const T & operator()(unsigned int x, unsigned int y) const
  {
    return _data ? (*_data)(x, y, _qp) : _value;
  }
};

template <typename T>
class GPUMaterialPropertyValue<T, 3> : public GPUMaterialPropertyValueBase<T, 3>
{
  usingGPUMaterialPropertyValueBaseMembers(T, 3);

public:
  KOKKOS_FUNCTION
  GPUMaterialPropertyValue(const GPUMaterialProperty<T, 3> & property,
                           SubdomainID sid,
                           dof_id_type qp)
    : GPUMaterialPropertyValueBase<T, 3>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x, unsigned int y, unsigned int z)
  {
    return (*_data)(x, y, z, _qp);
  }
  KOKKOS_FUNCTION const T & operator()(unsigned int x, unsigned int y, unsigned int z) const
  {
    return _data ? (*_data)(x, y, z, _qp) : _value;
  }
};

template <typename T>
class GPUMaterialPropertyValue<T, 4> : public GPUMaterialPropertyValueBase<T, 4>
{
  usingGPUMaterialPropertyValueBaseMembers(T, 4);

public:
  KOKKOS_FUNCTION
  GPUMaterialPropertyValue(const GPUMaterialProperty<T, 4> & property,
                           SubdomainID sid,
                           dof_id_type qp)
    : GPUMaterialPropertyValueBase<T, 4>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
  {
    return (*_data)(x, y, z, w, _qp);
  }
  KOKKOS_FUNCTION const T &
  operator()(unsigned int x, unsigned int y, unsigned int z, unsigned int w) const
  {
    return _data ? (*_data)(x, y, z, w, _qp) : _value;
  }
};

template <typename T, unsigned int dimension>
KOKKOS_FUNCTION GPUMaterialPropertyValue<T, dimension>
GPUMaterialProperty<T, dimension>::operator()(Datum & datum, unsigned int qp) const
{
  auto & elem = datum.elem();
  auto qp_offset = datum.qpOffset();

  return GPUMaterialPropertyValue<T, dimension>(*this, elem.subdomain, qp_offset + qp);
}
