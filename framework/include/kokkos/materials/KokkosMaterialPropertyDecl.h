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

#include "MoosePassKey.h"

#include <typeindex>

class MaterialBase;

namespace Moose::Kokkos
{

class MaterialPropertyStorage;

using StorageKey = Moose::PassKey<MaterialPropertyStorage>;

template <typename T, unsigned int dimension>
class MaterialPropertyValueBase;

template <typename T, unsigned int dimension>
class MaterialPropertyValue;

class Datum;
class Assembly;
class Mesh;

/**
 * Property constant options
 */
enum class PropertyConstantOption
{
  NONE,
  ELEMENT,
  SUBDOMAIN
};

/**
 * A structure storing the metadata of Kokkos material properties
 */
struct PropRecord
{
  /**
   * List of declaring materials
   */
  std::set<const ::MaterialBase *> declarers;
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
   * Size of each dimension of each subdomain
   */
  std::unordered_map<SubdomainID, std::vector<unsigned int>> dims;
  /**
   * Flag whether this property is a face property
   */
  bool bnd = false;
  /**
   * Flag whether this property is an on-demand property
   */
  bool on_demand = false;
  /**
   * Whether this property is constant over element or subdomain
   */
  PropertyConstantOption constant_option = PropertyConstantOption::NONE;
};

using PropertyStore = std::function<void(std::ostream &, void *)>;
using PropertyLoad = std::function<void(std::istream &, void *)>;

/**
 * The base class for Kokkos material properties
 */
class MaterialPropertyBase
{
public:
  /**
   * Default constructor
   */
  MaterialPropertyBase() = default;
  /**
   * Desturctor
   */
  virtual ~MaterialPropertyBase() {}

  /**
   * Get the property ID
   * @returns The property ID assigned by the MOOSE registry
   */
  unsigned int id() const { return _id; }
  /**
   * Get the property name
   * @returns The property name
   */
  const std::string & name() const;
  /**
   * Get the data type
   * @returns The demangled data type name
   */
  const std::string & type() const;
  /**
   * Get the dimension
   * @returns The dimension
   */
  unsigned int dim() const;
  /**
   * Get the size of a dimension
   * @param subdomain The MOOSE subdomain ID
   * @param i The dimension index
   * @returns The size of the dimension
   */
  unsigned int dimSize(SubdomainID subdomain, unsigned int i) const;

  /**
   * Get the property type index for load/store functions
   * @returns The property type index for the load/store function pointer map lookup
   */
  virtual std::type_index propertyType() = 0;

  /**
   * Initialize this property
   * @param record The record of this property
   */
  virtual void init(const PropRecord & record, const StorageKey &);

  /**
   * Allocate the data storage
   * @param mesh The Kokkos mesh
   * @param assembly The Kokkos assembly
   * @param subdomains The MOOSE subdomain IDs
   * @param bnd Whether this property is a face property
   */
  virtual void allocate(const Mesh & mesh,
                        const Assembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd,
                        StorageKey) = 0;
  /**
   * Deep copy another property
   * @param prop The property to copy
   */
  virtual void copy(const MaterialPropertyBase & prop, StorageKey) = 0;
  /**
   * Swap with another property
   * @param prop The property to swap
   */
  virtual void swap(MaterialPropertyBase & prop, StorageKey) = 0;

protected:
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
  /**
   * Whether this property is constant over element or subdomain
   */
  PropertyConstantOption _constant_option = PropertyConstantOption::NONE;
};

inline const std::string &
MaterialPropertyBase::name() const
{
  if (!_record)
    mooseError("Cannot get the name of an uninitialized or default material property.");
  else
    return _record->name;
}

inline const std::string &
MaterialPropertyBase::type() const
{
  if (!_record)
    mooseError("Cannot get the type of an uninitialized or default material property.");
  else
    return _record->type;
}

inline unsigned int
MaterialPropertyBase::dim() const
{
  if (!_record || !_record->dims.size())
    mooseError("Cannot get the dimension of an uninitialized or default material property.");
  else
    return _record->dims.begin()->second.size();
}

inline unsigned int
MaterialPropertyBase::dimSize(SubdomainID subdomain, unsigned int i) const
{
  const unsigned int D = dim();

  if (i >= D)
    mooseError("Cannot get the size of ",
               i,
               "-th dimension for the ",
               D,
               "D material property '",
               name(),
               "'.");

  return libmesh_map_find(_record->dims, subdomain)[i];
}

template <typename T, unsigned int dimension>
void propertyStore(std::ostream & stream, void * prop);
template <typename T, unsigned int dimension>
void propertyLoad(std::istream & stream, void * prop);

/**
 * The Kokkos material property class
 */
template <typename T, unsigned int dimension = 0>
class MaterialProperty final : public MaterialPropertyBase
{
public:
  /**
   * Default constructor
   */
  MaterialProperty() = default;
  /**
   * Constructor for default property
   * @param value The default value
   */
  MaterialProperty(const T & value);
  /**
   * Copy constructor
   * The reference material properties are held by the material property storage, and the user deals
   * with the clones of them. The reference material properties also hold the arrays for storing the
   * property values (_data), and the user accesses the arrays through their shallow copies in the
   * clones. As a result, if the reference material properties reallocate their arrays, the shallow
   * copies of arrays in the clones will lose synchronization. Thus, the clones also hold the
   * pointers to their reference material properties and shallow copy them in the copy constructor,
   * so that the arrays are always synchronized with those in the reference material properties
   * during parallel dispatch.
   */
  MaterialProperty(const MaterialProperty<T, dimension> & property);
  /**
   * Prevent initializing with properties of different rank
   */
  template <unsigned int D>
  MaterialProperty(const MaterialProperty<T, D> & other) = delete;

  /**
   * Shallow copy another property
   * @param property The property to be shallow copied
   */
  auto & operator=(const MaterialProperty<T, dimension> & property);

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether this property is valid
   * @returns Whether this property is valid
   */
  KOKKOS_FUNCTION operator bool() const { return _data.isAlloc() || _default; }

  /**
   * Get the property values of a quadrature point
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The MaterialPropertyValue object that provides access to the property values
   */
  KOKKOS_FUNCTION MaterialPropertyValue<T, dimension> operator()(const Datum & datum,
                                                                 const unsigned int qp) const;
#endif

  virtual std::type_index propertyType() override
  {
    static const std::type_index type = typeid(*this);

    return type;
  }

  virtual void init(const PropRecord & record, const StorageKey & key) override;

#ifdef MOOSE_KOKKOS_SCOPE
  virtual void allocate(const Mesh & mesh,
                        const Assembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd,
                        StorageKey) override;
  virtual void copy(const MaterialPropertyBase & prop, StorageKey) override;
  virtual void swap(MaterialPropertyBase & prop, StorageKey) override;
#endif

private:
  /**
   * Shallow copy another property
   * @param property The property to be shallow copied
   */
  void shallowCopy(const MaterialProperty<T, dimension> & property);

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

  friend class MaterialPropertyValueBase<T, dimension>;

  friend void propertyStore<T, dimension>(std::ostream &, void *);
  friend void propertyLoad<T, dimension>(std::istream &, void *);
};

// The Kokkos array containing Kokkos material properties requires a deep copy because the copy
// constructor of each Kokkos material property should be invoked
template <typename T, unsigned int dimension>
struct ArrayDeepCopy<MaterialProperty<T, dimension>>
{
  static constexpr bool value = true;
};

} // namespace Moose::Kokkos
