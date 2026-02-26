//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterialPropertyDecl.h"

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosMaterialPropertyValueDecl.h"
#endif

#include "KokkosAssembly.h"

#include "MooseMesh.h"

namespace Moose::Kokkos
{

inline void
MaterialPropertyBase::init(const PropRecord & record, const StorageKey &)
{
  _record = &record;
  _id = record.id;
  _constant_option = record.constant_option;
}

template <typename T, unsigned int dimension>
MaterialProperty<T, dimension>::MaterialProperty(const T & value)
{
  _default = true;
  _value = value;
}

template <typename T, unsigned int dimension>
MaterialProperty<T, dimension>::MaterialProperty(const MaterialProperty<T, dimension> & property)
{
  // If reference exists, copy the reference property
  // Reference can be nullptr if the property is a default or optional property
  const auto & prop = property._reference ? *property._reference : property;

  shallowCopy(prop);

  _reference = property._reference;
}

template <typename T, unsigned int dimension>
auto &
MaterialProperty<T, dimension>::operator=(const MaterialProperty<T, dimension> & property)
{
  shallowCopy(property);

  return *this;
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::init(const PropRecord & record, const StorageKey & key)
{
  MaterialPropertyBase::init(record, key);

  _reference = this;
}

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::copy(const MaterialPropertyBase & prop, StorageKey)
{
  auto prop_cast = dynamic_cast<const MaterialProperty<T, dimension> *>(&prop);

  mooseAssert(prop_cast, "The property to copy should be of the same type and dimension.");

  for (const auto i : index_range(prop_cast->_data))
    if (prop_cast->_data[i].isAlloc())
      _data[i].deepCopy(prop_cast->_data[i]);

  _data.copyToDevice();
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::swap(MaterialPropertyBase & prop, StorageKey)
{
  auto prop_cast = dynamic_cast<MaterialProperty<T, dimension> *>(&prop);

  mooseAssert(prop_cast, "The property to swap should be of the same type and dimension.");

  _data.swap(prop_cast->_data);
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::shallowCopy(const MaterialProperty<T, dimension> & property)
{
  _record = property._record;
  _id = property._id;
  _default = property._default;
  _constant_option = property._constant_option;

  _reference = property._reference;
  _data = property._data;
  _value = property._value;
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::allocate(const MooseMesh & mesh,
                                         const Assembly & assembly,
                                         const std::set<SubdomainID> & subdomains,
                                         const bool bnd,
                                         StorageKey)
{
  if (!_data.isAlloc())
    _data.create(mesh.nSubdomains());

  for (const auto subdomain : subdomains)
  {
    auto sid = mesh.getKokkosMesh()->getContiguousSubdomainID(subdomain);

    std::vector<dof_id_type> n;

    for (unsigned int i = 0; i < dimension; ++i)
      n.push_back(_record->dims[i]);

    if (_constant_option == PropertyConstantOption::NONE)
      n.push_back(bnd ? assembly.getNumFaceQps(sid) : assembly.getNumQps(sid));
    else if (_constant_option == PropertyConstantOption::ELEMENT)
      n.push_back(bnd ? assembly.getElemFacePropertySize(sid)
                      : mesh.getKokkosMesh()->getNumSubdomainLocalElements(subdomain));
    else
      n.push_back(1);

    if (!_data[sid].isAlloc())
      _data[sid].createDevice(n);
  }

  _data.copyToDevice();
}

template <typename T, unsigned int dimension>
KOKKOS_FUNCTION MaterialPropertyValue<T, dimension>
MaterialProperty<T, dimension>::operator()(const Datum & datum, const unsigned int qp) const
{
  return MaterialPropertyValue<T, dimension>(*this, datum, qp);
}
#endif

template <typename T, unsigned int dimension>
void
propertyStore(std::ostream & stream, void * prop)
{
  auto property = static_cast<MaterialProperty<T, dimension> *>(prop);

  dataStore(stream, property->_data, nullptr);
}
template <typename T, unsigned int dimension>
void
propertyLoad(std::istream & stream, void * prop)
{
  auto property = static_cast<MaterialProperty<T, dimension> *>(prop);

  dataLoad(stream, property->_data, nullptr);
}

} // namespace Moose::Kokkos
