//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

#include "MooseADWrapper.h"
#include "MooseArray.h"
#include "MooseTypes.h"
#include "DataIO.h"
#include "MooseError.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

class PropertyValue;
class Material;
class MaterialPropertyInterface;

/**
 * Scalar Init helper routine so that specialization isn't needed for basic scalar MaterialProperty
 * types
 */
template <typename T>
PropertyValue * _init_helper(int size, T * prop);

/**
 * Abstract definition of a property value.
 */
class PropertyValue
{
public:
  virtual ~PropertyValue(){};

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() = 0;

  /**
   * Clone this value.  Useful in copy-construction.
   */
  virtual PropertyValue * init(int size) = 0;

  virtual unsigned int size() const = 0;

  /**
   * Resizes the property to the size n
   */
  virtual void resize(int n) = 0;

  virtual void swap(PropertyValue * rhs) = 0;

  virtual bool isAD() = 0;

  /**
   * Copy the value of a Property from one specific to a specific qp in this Property.
   * Important note: this copy operation loses AD derivative information if either this
   * or the rhs is not an AD material property
   *
   * @param to_qp The quadrature point in _this_ Property that you want to copy to.
   * @param rhs The Property you want to copy _from_.
   * @param from_qp The quadrature point in rhs you want to copy _from_.
   */
  virtual void
  qpCopy(const unsigned int to_qp, PropertyValue * rhs, const unsigned int from_qp) = 0;

  // save/restore in a file
  virtual void store(std::ostream & stream) = 0;
  virtual void load(std::istream & stream) = 0;
};

template <>
inline void
dataStore(std::ostream & stream, PropertyValue *& p, void * /*context*/)
{
  p->store(stream);
}

template <>
inline void
dataLoad(std::istream & stream, PropertyValue *& p, void * /*context*/)
{
  p->load(stream);
}

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T, bool is_ad>
class MaterialPropertyBase : public PropertyValue
{
public:
  typedef MooseADWrapper<T, is_ad> value_type;

  /// Explicitly declare a public constructor because we made the copy constructor private
  MaterialPropertyBase() : PropertyValue()
  { /* */
  }

  virtual ~MaterialPropertyBase() {}

  bool isAD() override { return is_ad; }

  /**
   * @returns a read-only reference to the parameter value.
   */
  const MooseArray<MooseADWrapper<T, is_ad>> & get() const { return _value; }

  /**
   * @returns a writable reference to the parameter value.
   */
  MooseArray<MooseADWrapper<T, is_ad>> & set() { return _value; }

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() override;

  /**
   * Resizes the property to the size n
   */
  virtual void resize(int n) override;

  virtual unsigned int size() const override { return _value.size(); }

  /**
   * Get element i out of the array as a writeable reference.
   */
  MooseADWrapper<T, is_ad> & operator[](const unsigned int i) { return _value[i]; }

  /**
   * Get element i out of the array as a ready-only reference.
   */
  const MooseADWrapper<T, is_ad> & operator[](const unsigned int i) const { return _value[i]; }

  /**
   * Copy the value of a Property from one specific to a specific qp in this Property.
   *
   * @param to_qp The quadrature point in _this_ Property that you want to copy to.
   * @param rhs The Property you want to copy _from_.
   * @param from_qp The quadrature point in rhs you want to copy _from_.
   */
  virtual void
  qpCopy(const unsigned int to_qp, PropertyValue * rhs, const unsigned int from_qp) override;

  /**
   * Store the property into a binary stream
   */
  virtual void store(std::ostream & stream) override;

  /**
   * Load the property from a binary stream
   */
  virtual void load(std::istream & stream) override;

  void setName(const MaterialPropertyName & name_in)
  {
    mooseAssert(
        _name.empty() || _name == name_in,
        "We're trying to apply a new name to a material property. I don't think that makes sense.");
    _name = name_in;
  }

  const MaterialPropertyName & name() const
  {
    if (_name.empty())
      mooseError("Retrieving a material property name before it's set.");
    return _name;
  }

  void swap(PropertyValue * rhs) override;

private:
  /// private copy constructor to avoid shallow copying of material properties
  MaterialPropertyBase(const MaterialPropertyBase<T, is_ad> & /*src*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }

  /// private assignment operator to avoid shallow copying of material properties
  MaterialPropertyBase<T, is_ad> & operator=(const MaterialPropertyBase<T, is_ad> & /*rhs*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }

  /// the name of this material property
  MaterialPropertyName _name;

protected:
  /// Stored parameter value.
  MooseArray<MooseADWrapper<T, is_ad>> _value;
};

template <typename T>
class MaterialProperty;
template <typename T>
class ADMaterialProperty;

// ------------------------------------------------------------
// Material::Property<> class inline methods

namespace moose
{
namespace internal
{
template <typename T1, typename T2>
void
rawValueEqualityHelper(T1 & out, const T2 & in)
{
  out = MetaPhysicL::raw_value(in);
}

template <typename T1, typename T2>
void
rawValueEqualityHelper(std::vector<T1> & out, const std::vector<T2> & in)
{
  out.resize(in.size());
  for (MooseIndex(in) i = 0; i < in.size(); ++i)
    rawValueEqualityHelper(out[i], in[i]);
}

template <typename T1, typename T2, std::size_t N>
void
rawValueEqualityHelper(std::array<T1, N> & out, const std::array<T2, N> & in)
{
  for (MooseIndex(in) i = 0; i < in.size(); ++i)
    rawValueEqualityHelper(out[i], in[i]);
}
}
}

template <typename T, bool is_ad>
inline std::string
MaterialPropertyBase<T, is_ad>::type()
{
  return typeid(MooseADWrapper<T, is_ad>).name();
}

template <typename T, bool is_ad>
inline void
MaterialPropertyBase<T, is_ad>::resize(int n)
{
  _value.template resize</*value_initalize=*/true>(n);
}

template <typename T, bool is_ad>
inline void
MaterialPropertyBase<T, is_ad>::qpCopy(const unsigned int to_qp,
                                       PropertyValue * rhs,
                                       const unsigned int from_qp)
{
  mooseAssert(rhs != NULL, "Assigning NULL?");

  // If we're the same
  if (rhs->isAD() == is_ad)
    _value[to_qp] = cast_ptr<const MaterialPropertyBase<T, is_ad> *>(rhs)->_value[from_qp];

  else
    moose::internal::rawValueEqualityHelper(
        _value[to_qp], (*cast_ptr<const MaterialPropertyBase<T, !is_ad> *>(rhs))[from_qp]);
}

template <typename T, bool is_ad>
inline void
MaterialPropertyBase<T, is_ad>::store(std::ostream & stream)
{
  for (unsigned int i = 0; i < size(); i++)
    storeHelper(stream, _value[i], NULL);
}

template <typename T, bool is_ad>
inline void
MaterialPropertyBase<T, is_ad>::load(std::istream & stream)
{
  for (unsigned int i = 0; i < size(); i++)
    loadHelper(stream, _value[i], NULL);
}

template <typename T, bool is_ad>
inline void
MaterialPropertyBase<T, is_ad>::swap(PropertyValue * rhs)
{
  mooseAssert(rhs, "Assigning nullptr?");

  // If we're the same
  if (rhs->isAD() == is_ad)
  {
    this->_value.swap(cast_ptr<MaterialPropertyBase<T, is_ad> *>(rhs)->_value);
    return;
  }

  // We may call this function when doing swap between MaterialData material properties (you can
  // think of these as the current element properties) and MaterialPropertyStorage material
  // properties (these are the stateful material properties that we store for *every* element). We
  // never store ADMaterialProperty in stateful storage (e.g. MaterialPropertyStorage) for memory
  // resource reasons; instead we keep a regular MaterialProperty version of it. Hence we do have a
  // need to exchange data between the AD and regular copies which we implement below. The below
  // is obviously not a swap, for which you cannot identify a giver and receiver. Instead the below
  // has a clear giver and receiver. The giver is the object passed in as the rhs. The receiver is
  // *this* object. This directionality, although not conceptually appropriate given the method
  // name, *is* appropriate to how this method is used in practice. See shallowCopyData and
  // shallowCopyDataBack in MaterialPropertyStorage.C

  auto * different_type_prop = dynamic_cast<MaterialPropertyBase<T, !is_ad> *>(rhs);
  mooseAssert(different_type_prop,
              "Wrong material property type T in MaterialPropertyBase<T, is_ad>::swap");

  this->resize(different_type_prop->size());
  for (const auto qp : make_range(this->size()))
    moose::internal::rawValueEqualityHelper(this->_value[qp], (*different_type_prop)[qp]);
}

template <typename T>
class MaterialProperty : public MaterialPropertyBase<T, false>
{
public:
  MaterialProperty() = default;

  PropertyValue * init(int size) override { return _init_helper(size, this); }

private:
  /// private copy constructor to avoid shallow copying of material properties
  MaterialProperty(const MaterialProperty<T> & /*src*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }

  /// private assignment operator to avoid shallow copying of material properties
  MaterialProperty<T> & operator=(const MaterialProperty<T> & /*rhs*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }
};

template <typename T>
class ADMaterialProperty : public MaterialPropertyBase<T, true>
{
public:
  ADMaterialProperty() = default;

  using typename MaterialPropertyBase<T, true>::value_type;

  PropertyValue * init(int size) override { return _init_helper(size, this); }

private:
  /// private copy constructor to avoid shallow copying of material properties
  ADMaterialProperty(const ADMaterialProperty<T> & /*src*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }

  /// private assignment operator to avoid shallow copying of material properties
  ADMaterialProperty<T> & operator=(const ADMaterialProperty<T> & /*rhs*/)
  {
    mooseError("Material properties must be assigned to references (missing '&')");
  }
};

/**
 * Container for storing material properties
 */
class MaterialProperties : public std::vector<PropertyValue *>
{
public:
  MaterialProperties() {}

  virtual ~MaterialProperties() {}

  /**
   * Parameter map iterator.
   */
  typedef std::vector<PropertyValue *>::iterator iterator;

  /**
   * Constant parameter map iterator.
   */
  typedef std::vector<PropertyValue *>::const_iterator const_iterator;

  /**
   * Deallocates the memory
   */
  void destroy()
  {
    for (iterator k = begin(); k != end(); ++k)
      delete *k;
  }

  /**
   * Resize items in this array, i.e. the number of values needed in PropertyValue array
   * @param n_qpoints The number of values needed to store (equals the the number of quadrature
   * points per mesh element)
   */
  void resizeItems(unsigned int n_qpoints)
  {
    for (iterator k = begin(); k != end(); ++k)
      if (*k != NULL)
        (*k)->resize(n_qpoints);
  }
};

template <>
inline void
dataStore(std::ostream & stream, MaterialProperties & v, void * context)
{
  // Cast this to a vector so we can just piggy back on the vector store capability
  std::vector<PropertyValue *> & mat_props = static_cast<std::vector<PropertyValue *> &>(v);

  storeHelper(stream, mat_props, context);
}

template <>
inline void
dataLoad(std::istream & stream, MaterialProperties & v, void * context)
{
  // Cast this to a vector so we can just piggy back on the vector store capability
  std::vector<PropertyValue *> & mat_props = static_cast<std::vector<PropertyValue *> &>(v);

  // But the vector store capability should not be changing the vector
  // size
#ifndef NDEBUG
  const std::size_t old_size = mat_props.size();
#endif
  loadHelper(stream, mat_props, context);
  mooseAssert(old_size == mat_props.size(),
              "Loading MaterialProperties data into mis-sized target");
}

// Scalar Init Helper Function
template <template <typename> class DerivedMaterialProperty, typename T>
PropertyValue *
_init_helper(int size, DerivedMaterialProperty<T> * /*prop*/)
{
  // This function is used to initialize stateful material properties. We *never* want to create a
  // stateful AD material property because it is too memory intensive
  MaterialProperty<T> * copy = new MaterialProperty<T>;
  copy->resize(size);
  return copy;
}

template <typename T, bool is_ad>
struct GenericMaterialPropertyStruct
{
  typedef MaterialProperty<T> type;
};

template <typename T>
struct GenericMaterialPropertyStruct<T, true>
{
  typedef ADMaterialProperty<T> type;
};

template <typename T, bool is_ad>
using GenericMaterialProperty = typename GenericMaterialPropertyStruct<T, is_ad>::type;

/**
 * Base class to facilitate storage using unique pointers
 */
class GenericOptionalMaterialPropertyBase
{
public:
  virtual ~GenericOptionalMaterialPropertyBase() {}
};

template <class M, typename T, bool is_ad>
class OptionalMaterialPropertyProxy;

/**
 * Wrapper around a material property pointer. Copying this wrapper is disabled
 * to enforce capture via reference. Used by the optional material property
 * API, which requires late binding updates of the stored pointer.
 */
template <typename T, bool is_ad>
class GenericOptionalMaterialProperty : public GenericOptionalMaterialPropertyBase
{
  typedef GenericMaterialProperty<T, is_ad> P;

public:
  GenericOptionalMaterialProperty(const P * pointer) : _pointer(pointer) {}

  /// no copy construction is permitted
  GenericOptionalMaterialProperty(const GenericOptionalMaterialProperty<T, is_ad> &) = delete;
  /// no copy assignment is permitted
  GenericOptionalMaterialProperty &
  operator=(const GenericOptionalMaterialProperty<T, is_ad> &) = delete;

  /// pass through operator[] to provide a similar API as MaterialProperty
  const MooseADWrapper<T, is_ad> & operator[](const unsigned int i) const
  {
    // check if the optional property is valid in debug mode
    mooseAssert(
        _pointer,
        "Attempting to access an optional material property that was not provided by any material "
        "class. Make sure to check optional material properties before using them.");
    return (*_pointer)[i];
  }

  /// pass through size calls
  unsigned int size() const { return (*_pointer).size(); }

  /// implicit cast to bool to check the if the material property exists
  operator bool() const { return _pointer; }

  /// get a pointer to the underlying property (only do this in initialSetup or later)
  const P * get() const { return _pointer; }

private:
  /// the default constructor is only called from the friend class
  GenericOptionalMaterialProperty() : _pointer(nullptr) {}

  /// setting the pointer is only permitted through the optional material proxy system
  void set(const P * pointer) { _pointer = pointer; }
  const P * _pointer;

  friend class OptionalMaterialPropertyProxy<Material, T, is_ad>;
  friend class OptionalMaterialPropertyProxy<MaterialPropertyInterface, T, is_ad>;
};

template <typename T>
using OptionalMaterialProperty = GenericOptionalMaterialProperty<T, false>;
template <typename T>
using OptionalADMaterialProperty = GenericOptionalMaterialProperty<T, true>;
