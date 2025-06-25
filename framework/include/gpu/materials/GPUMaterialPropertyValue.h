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

#define usingKokkosMaterialPropertyValueBaseMembers(T, dimension)                                  \
  using MaterialPropertyValueBase<T, dimension>::_qp;                                              \
  using MaterialPropertyValueBase<T, dimension>::_data;                                            \
  using MaterialPropertyValueBase<T, dimension>::_value;

namespace Moose
{
namespace Kokkos
{

template <typename T, unsigned int dimension>
class MaterialPropertyValueBase
{
protected:
  // Current quadrature point
  const dof_id_type _qp;
  // Data array
  const Array<T, dimension + 1> * _data;
  // Default value
  const T & _value;

public:
  KOKKOS_FUNCTION MaterialPropertyValueBase(const MaterialProperty<T, dimension> & property,
                                            SubdomainID sid,
                                            dof_id_type qp)
    : _qp(qp), _data(property._default ? nullptr : &property._data[sid]), _value(property._value)
  {
  }
  // Get the size of each dimension
  KOKKOS_FUNCTION uint64_t n(unsigned int dim) { return _data->n(dim); }
};

template <typename T, unsigned int dimension>
class MaterialPropertyValue
{
};

template <typename T>
class MaterialPropertyValue<T, 0> : public MaterialPropertyValueBase<T, 0>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 0);

public:
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 0> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 0>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION operator const T &() const { return _data ? (*_data)(_qp) : _value; }
  KOKKOS_FUNCTION auto & operator=(const T & data)
  {
    (*_data)(_qp) = data;

    return *this;
  }
  KOKKOS_FUNCTION auto & operator=(const MaterialPropertyValue<T, 0> & property)
  {
    (*_data)(_qp) = static_cast<T>(property);

    return *this;
  }
};

template <typename T>
class MaterialPropertyValue<T, 1> : public MaterialPropertyValueBase<T, 1>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 1);

public:
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 1> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 1>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x) { return (*_data)(x, _qp); }
  KOKKOS_FUNCTION const T & operator()(unsigned int x) const
  {
    return _data ? (*_data)(x, _qp) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 2> : public MaterialPropertyValueBase<T, 2>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 2);

public:
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 2> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 2>(property, sid, qp)
  {
  }
  KOKKOS_FUNCTION T & operator()(unsigned int x, unsigned int y) { return (*_data)(x, y, _qp); }
  KOKKOS_FUNCTION const T & operator()(unsigned int x, unsigned int y) const
  {
    return _data ? (*_data)(x, y, _qp) : _value;
  }
};

template <typename T>
class MaterialPropertyValue<T, 3> : public MaterialPropertyValueBase<T, 3>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 3);

public:
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 3> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 3>(property, sid, qp)
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
class MaterialPropertyValue<T, 4> : public MaterialPropertyValueBase<T, 4>
{
  usingKokkosMaterialPropertyValueBaseMembers(T, 4);

public:
  KOKKOS_FUNCTION
  MaterialPropertyValue(const MaterialProperty<T, 4> & property, SubdomainID sid, dof_id_type qp)
    : MaterialPropertyValueBase<T, 4>(property, sid, qp)
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
KOKKOS_FUNCTION MaterialPropertyValue<T, dimension>
MaterialProperty<T, dimension>::operator()(Datum & datum, unsigned int qp) const
{
  auto & elem = datum.elem();
  auto qp_offset = datum.qpOffset();

  return MaterialPropertyValue<T, dimension>(*this, elem.subdomain, qp_offset + qp);
}

} // namespace Kokkos
} // namespace Moose
