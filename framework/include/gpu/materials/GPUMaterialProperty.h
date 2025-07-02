//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUAssembly.h"

#include "MooseMesh.h"

namespace Moose
{
namespace Kokkos
{

class MaterialPropertyStorage;

template <typename T, unsigned int dimension>
class MaterialPropertyValueBase;

template <typename T, unsigned int dimension>
class MaterialPropertyValue;

class Datum;

/**
 * A structure storing the metadata of Kokkos material properties
 */
struct PropRecord
{
  /**
   * List of declaring materials
   */
  std::set<const MaterialBase *> declarers;
  /**
   * Property name
   */
  std::string name;
  /**
   * Demangled data type name
   */
  std::string type;
  /**
   * Property ID
   */
  unsigned int id = libMesh::invalid_uint;
  /**
   * Size of each dimension
   */
  std::vector<unsigned int> dims;
  /**
   * Flag whether this property is a face property
   */
  bool bnd = false;
};

using PropertyKey = std::pair<std::type_index, unsigned int>;
using PropertyStore = std::function<void(std::ostream &, void *)>;
using PropertyLoad = std::function<void(std::istream &, void *)>;

/**
 * The base class for Kokkos material properties
 */
class MaterialPropertyBase
{
  friend class MaterialPropertyStorage;

public:
  /**
   * Default constructor
   */
  MaterialPropertyBase() = default;

  /**
   * Get the property ID
   * @returns The property ID assigned by the MOOSE registry
   */
  auto id() const { return _id; }
  /**
   * Get the property name
   * @returns The property name
   */
  auto name() const { return _record->name; }
  /**
   * Get the data type
   * @returns The demangled data type name
   */
  auto type() const { return _record->type; }
  /**
   * Get the dimension
   * @returns The dimension
   */
  auto dim() const { return _record->dims.size(); }
  /**
   * Get the size of a dimension
   * @param i The dimension index
   * @returns The size of the dimension
   */
  auto dim(unsigned int i) const
  {
    if (i >= dim())
      mooseError("Cannot query the size of ",
                 i,
                 "-th dimension for the ",
                 dim(),
                 "D material property '",
                 name(),
                 "'.");

    return _record->dims.at(i);
  }

  /**
   * Store the data to a stream
   * @param stream The stream to store
   */
  void store(std::ostream & stream) { _store[key()](stream, this); }
  /**
   * Load the data from a stream
   * @param stream The stream to load
   */
  void load(std::istream & stream) { _load[key()](stream, this); }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether this property is valid
   * @returns Whether this property is valid
   */
  KOKKOS_FUNCTION operator bool() const { return _id != libMesh::invalid_uint || _default; }
#endif

protected:
  /**
   * Function pointer maps for load/store
   */
  ///{@
  static std::unordered_map<PropertyKey, PropertyStore> _store;
  static std::unordered_map<PropertyKey, PropertyLoad> _load;
  ///@}

  /**
   * Get the key to call load/store functions
   * @returns The key for the load/store function pointer map
   */
  virtual PropertyKey key() = 0;

  /**
   * Pointer to the record of this property
   */
  const PropRecord * _record = nullptr;
  /**
   * Property ID
   */
  unsigned int _id = libMesh::invalid_uint;
  /**
   * Flag whether this property has a default value
   */
  bool _default = false;

private:
  /**
   * Initialize this property
   * @param record The record of this property
   */
  void init(const PropRecord & record)
  {
    _record = &record;
    _id = record.id;
  }

  /**
   * Allocate the data storage
   * @param mesh The MOOSE mesh
   * @param assembly The Kokkos assembly
   * @param subdomains The MOOSE subdomain IDs
   * @param bnd Whether this property is a face property
   */
  virtual void allocate(const MooseMesh & mesh,
                        const Assembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd) = 0;
  /**
   * Deep copy another property
   * @param prop The property to copy
   */
  virtual void copy(const MaterialPropertyBase & prop) = 0;
  /**
   * Swap with another property
   * @param prop The property to swap
   */
  virtual void swap(MaterialPropertyBase & prop) = 0;
};

template <typename T, unsigned int dimension>
void propertyStore(std::ostream & stream, void * prop);
template <typename T, unsigned int dimension>
void propertyLoad(std::istream & stream, void * prop);

/**
 * The Kokkos material property class
 */
template <typename T, unsigned int dimension = 0>
class MaterialProperty : public MaterialPropertyBase
{
  friend class MaterialPropertyValueBase<T, dimension>;

  friend void propertyStore<T, dimension>(std::ostream &, void *);
  friend void propertyLoad<T, dimension>(std::istream &, void *);

public:
  /**
   * Default constructor
   */
  MaterialProperty() = default;
  /**
   * Constructor for default property
   * @param value The default value
   */
  MaterialProperty(const T value)
  {
    _default = true;
    _value = value;
  }
  /**
   * Copy constructor
   */
  MaterialProperty(const MaterialProperty<T, dimension> & property)
  {
    if (!property._reference)
    {
      *this = property;
      this->_reference = &property;
    }
    else
    {
      *this = *property._reference;
      this->_reference = property._reference;
    }
  }

  /**
   * Get the clone of this property
   * @returns The clone this property constructed through the copy constructor
   */
  MaterialProperty<T, dimension> clone() { return MaterialProperty<T, dimension>(*this); }

  /**
   * Register the load/store functions for this property type
   */
  void registerLoadStore()
  {
    _store[key()] = propertyStore<T, dimension>;
    _load[key()] = propertyLoad<T, dimension>;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the property values of a quadrature point
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The MaterialPropertyValue object that provides access to the property values
   */
  KOKKOS_FUNCTION MaterialPropertyValue<T, dimension> operator()(Datum & datum,
                                                                 unsigned int qp) const;
#endif

protected:
  virtual PropertyKey key() override
  {
    return std::make_pair(std::type_index(typeid(T)), dimension);
  }

private:
#ifdef MOOSE_KOKKOS_SCOPE
  virtual void allocate(const MooseMesh & mesh,
                        const Assembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd) override;
  virtual void copy(const MaterialPropertyBase & prop) override;
  virtual void swap(MaterialPropertyBase & prop) override;
#endif

  /**
   * Pointer to the reference property
   */
  const MaterialProperty<T, dimension> * _reference = nullptr;
  /**
   * Data storage
   */
  Array<Array<T, dimension + 1>> _data;
  /**
   * Default value
   */
  T _value;
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::copy(const MaterialPropertyBase & prop)
{
  auto & prop_cast = static_cast<const MaterialProperty<T, dimension> &>(prop);

  for (uint64_t i = 0; i < prop_cast._data.size(); ++i)
    if (prop_cast._data[i].isAlloc())
      _data[i].deepCopy(prop_cast._data[i]);

  _data.copy();
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::swap(MaterialPropertyBase & prop)
{
  auto & prop_cast = static_cast<MaterialProperty<T, dimension> &>(prop);

  _data.swap(prop_cast._data);
}

template <typename T, unsigned int dimension>
void
MaterialProperty<T, dimension>::allocate(const MooseMesh & mesh,
                                         const Assembly & assembly,
                                         const std::set<SubdomainID> & subdomains,
                                         const bool bnd)
{
  if (!_data.isAlloc())
    _data.create(mesh.meshSubdomains().size());

  for (auto subdomain : subdomains)
  {
    auto sid = mesh.getKokkosMesh()->getSubdomainID(subdomain);

    std::vector<uint64_t> n;

    for (unsigned int i = 0; i < dimension; ++i)
      n.push_back(_record->dims[i]);

    n.push_back(bnd ? assembly.getNumFaceQps(sid) : assembly.getNumQps(sid));

    if (!_data[sid].isAlloc())
      _data[sid].createDevice(n);
  }

  _data.copy();
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

// The Kokkos array containing Kokkos material properties requires a deep copy because the copy
// constructor of each Kokkos material property should be invoked
template <typename T>
struct ArrayDeepCopy<MaterialProperty<T>>
{
  static const bool value = true;
};

} // namespace Kokkos
} // namespace Moose
