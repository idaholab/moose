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

class MooseMesh;
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
   * Size of each dimension
   */
  std::vector<unsigned int> dims;
  /**
   * Flag whether this property is a face property
   */
  bool bnd = false;
  /**
   * Flag whether this property is a lazy property
   */
  bool lazy = false;
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
  std::string name() const { return _record->name; }
  /**
   * Get the data type
   * @returns The demangled data type name
   */
  std::string type() const { return _record->type; }
  /**
   * Get the dimension
   * @returns The dimension
   */
  unsigned int dim() const { return _record->dims.size(); }
  /**
   * Get the size of a dimension
   * @param i The dimension index
   * @returns The size of the dimension
   */
  unsigned int dim(unsigned int i) const
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
   * @param mesh The MOOSE mesh
   * @param assembly The Kokkos assembly
   * @param subdomains The MOOSE subdomain IDs
   * @param bnd Whether this property is a face property
   */
  virtual void allocate(const MooseMesh & mesh,
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
};

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
  virtual void allocate(const MooseMesh & mesh,
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
