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
#include "DataIO.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

#include "metaphysicl/raw_type.h"

class PropertyValue;

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
   * Creates a regular material property, copying any existing qp values from this
   */
  virtual PropertyValue * makeRegularProperty() = 0;

  /**
   * Creates an AD material property, copying any existing qp *values* from this. Derivative
   * information is zeroed
   */
  virtual PropertyValue * makeADProperty() = 0;

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

  PropertyValue * makeRegularProperty() override;
  PropertyValue * makeADProperty() override;

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
   *
   */
  virtual void swap(PropertyValue * rhs) override;

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
    out[i] = MetaPhysicL::raw_value(in[i]);
}

template <typename T1, typename T2>
void
rawValueEqualityHelper(std::vector<std::vector<T1>> & out, const std::vector<std::vector<T2>> & in)
{
  out.resize(in.size());
  for (MooseIndex(in) i = 0; i < in.size(); ++i)
  {
    out[i].resize(in[i].size());
    for (MooseIndex(in[i].size()) j = 0; j < in[i].size(); ++j)
      out[i][j] = MetaPhysicL::raw_value(in[i][j]);
  }
}
}
}

template <typename T, bool is_ad>
PropertyValue *
MaterialPropertyBase<T, is_ad>::makeRegularProperty()
{
  auto * new_prop = new MaterialProperty<T>;

  new_prop->resize(this->size());

  for (MooseIndex(_value) i = 0; i < _value.size(); ++i)
    moose::internal::rawValueEqualityHelper((*new_prop)[i], _value[i]);

  return new_prop;
}

template <typename T, bool is_ad>
PropertyValue *
MaterialPropertyBase<T, is_ad>::makeADProperty()
{
  auto * new_prop = new ADMaterialProperty<T>;

  new_prop->resize(this->size());

  for (MooseIndex(_value) i = 0; i < _value.size(); ++i)
    moose::internal::rawValueEqualityHelper((*new_prop)[i], _value[i]);

  return new_prop;
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
MaterialPropertyBase<T, is_ad>::swap(PropertyValue * rhs)
{
  mooseAssert(rhs != NULL, "Assigning NULL?");
  _value.swap(cast_ptr<MaterialPropertyBase<T, is_ad> *>(rhs)->_value);
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

  loadHelper(stream, mat_props, context);
}

// Scalar Init Helper Function
template <typename T>
PropertyValue *
_init_helper(int size, T * /*prop*/)
{
  T * copy = new T;
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
