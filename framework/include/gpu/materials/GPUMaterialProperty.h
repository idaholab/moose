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

class GPUMaterialPropertyStorage;

template <typename T, unsigned int dimension>
class GPUMaterialPropertyValueBase;

template <typename T, unsigned int dimension>
class GPUMaterialPropertyValue;

class Datum;

struct GPUPropRecord
{
  /// The declaring materials of this property
  std::set<const MaterialBase *> declarers;
  /// The name of this property
  std::string name;
  /// The pretty type name of this property
  std::string type;
  /// The ID of this property
  unsigned int id = libMesh::invalid_uint;
  /// The dimension of this property
  std::vector<unsigned int> dims;
  /// Whether or not this property is a boundary property
  bool bnd = false;
};

using GPUPropertyKey = std::pair<std::type_index, unsigned int>;
using GPUPropertyStore = std::function<void(std::ostream &, void *)>;
using GPUPropertyLoad = std::function<void(std::istream &, void *)>;

class GPUMaterialPropertyBase
{
  friend class GPUMaterialPropertyStorage;

protected:
  // Shared function pointer map for propertyStore and propertyLoad
  static std::unordered_map<GPUPropertyKey, GPUPropertyStore> _store;
  static std::unordered_map<GPUPropertyKey, GPUPropertyLoad> _load;

  // Key to access data functions
  virtual GPUPropertyKey key() = 0;

protected:
  // Pointer to the record
  const GPUPropRecord * _record = nullptr;
  // The property ID
  unsigned int _id = libMesh::invalid_uint;
  // Whether this property has a default value
  bool _default = false;

public:
  // Get the property ID
  auto id() const { return _id; }
  // Get the property name
  auto name() const { return _record->name; }
  // Get the data type
  auto type() const { return _record->type; }
  // Get the dimension
  auto dim() const { return _record->dims.size(); }
  // Get the size of a dimension
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

private:
#ifdef MOOSE_GPU_SCOPE
  void init(const GPUPropRecord & record)
  {
    _record = &record;
    _id = record.id;
  }
#endif

  // Allocate data
  virtual void allocate(const MooseMesh & mesh,
                        const GPUAssembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd) = 0;

public:
#ifdef MOOSE_GPU_SCOPE
  // Whether this material property is valid
  KOKKOS_FUNCTION operator bool() const { return _id != libMesh::invalid_uint || _default; }

  // Store material property to stream
  void store(std::ostream & stream) { _store[key()](stream, this); }

  // Load material property from stream
  void load(std::istream & stream) { _load[key()](stream, this); }
#endif

  // Deep copy another material property
  virtual void copy(const GPUMaterialPropertyBase & prop) = 0;

  // Swap two material properties
  virtual void swap(GPUMaterialPropertyBase & prop) = 0;
};

template <typename T, unsigned int dimension>
void propertyStore(std::ostream & stream, void * prop);
template <typename T, unsigned int dimension>
void propertyLoad(std::istream & stream, void * prop);

template <typename T, unsigned int dimension = 0>
class GPUMaterialProperty : public GPUMaterialPropertyBase
{
  friend class GPUMaterialPropertyValueBase<T, dimension>;

  friend void propertyStore<T, dimension>(std::ostream &, void *);
  friend void propertyLoad<T, dimension>(std::istream &, void *);

protected:
  // Key to access data functions
  virtual GPUPropertyKey key() override
  {
    return std::make_pair(std::type_index(typeid(T)), dimension);
  }

public:
  GPUMaterialProperty() {}
  // Constructor for default material property
  GPUMaterialProperty(const T value)
  {
    _default = true;
    _value = value;
  }

private:
  // Reference property
  const GPUMaterialProperty<T, dimension> * _reference = nullptr;
  // Data array
  GPUArray<GPUArray<T, dimension + 1>> _data;
  // Default value
  T _value;

#ifdef MOOSE_GPU_SCOPE
public:
  // Register the data functions for this type
  void registerLoadStore()
  {
    _store[key()] = propertyStore<T, dimension>;
    _load[key()] = propertyLoad<T, dimension>;
  }

private:
  // Allocate data
  virtual void allocate(const MooseMesh & mesh,
                        const GPUAssembly & assembly,
                        const std::set<SubdomainID> & subdomains,
                        const bool bnd) override
  {
    if (!_data.isAlloc())
      _data.create(mesh.meshSubdomains().size());

    for (auto subdomain : subdomains)
    {
      auto sid = mesh.getGPUMesh()->getGPUSubdomainID(subdomain);

      std::vector<uint64_t> n;

      for (unsigned int i = 0; i < dimension; ++i)
        n.push_back(_record->dims[i]);

      n.push_back(bnd ? assembly.getNumFaceQps(sid) : assembly.getNumQps(sid));

      if (!_data[sid].isAlloc())
        _data[sid].createDevice(n);
    }

    _data.copy();
  }

public:
  // Copy constructor
  GPUMaterialProperty(const GPUMaterialProperty<T, dimension> & property)
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

  // Deep copy another material property
  virtual void copy(const GPUMaterialPropertyBase & prop) override
  {
    auto & prop_cast = static_cast<const GPUMaterialProperty<T, dimension> &>(prop);

    for (uint64_t i = 0; i < prop_cast._data.size(); ++i)
      if (prop_cast._data[i].isAlloc())
        _data[i].deepCopy(prop_cast._data[i]);

    _data.copy();
  }

  // Swap two material properties
  virtual void swap(GPUMaterialPropertyBase & prop) override
  {
    auto & prop_cast = static_cast<GPUMaterialProperty<T, dimension> &>(prop);

    _data.swap(prop_cast._data);
  }

  // Get the mirror property
  GPUMaterialProperty<T, dimension> mirror() { return GPUMaterialProperty<T, dimension>(*this); }
  // Get the quadrature point data
  KOKKOS_FUNCTION GPUMaterialPropertyValue<T, dimension> operator()(Datum & datum,
                                                                    unsigned int qp) const;
#endif
};

template <typename T, unsigned int dimension>
void
propertyStore(std::ostream & stream, void * prop)
{
  auto property = static_cast<GPUMaterialProperty<T, dimension> *>(prop);

  dataStore(stream, property->_data, nullptr);
}
template <typename T, unsigned int dimension>
void
propertyLoad(std::istream & stream, void * prop)
{
  auto property = static_cast<GPUMaterialProperty<T, dimension> *>(prop);

  dataLoad(stream, property->_data, nullptr);
}

template <typename T>
struct GPUArrayDeepCopy<GPUMaterialProperty<T>>
{
  static const bool value = true;
};
