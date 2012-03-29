/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALPROPERTY_H
#define MATERIALPROPERTY_H

#include <vector>

#include "MooseArray.h"
#include "ColumnMajorMatrix.h"

#include "libmesh_common.h"
#include "tensor_value.h"
#include "vector_value.h"


/**
 * Abstract definition of a property value.
 */
class PropertyValue
{
public:
  /**
   * Destructor.
   */
  virtual ~PropertyValue() {};

  /**
   * String identifying the type of parameter stored.
   * Must be reimplemented in derived classes.
   */
  virtual std::string type () = 0;

  /**
   * Clone this value.  Useful in copy-construction.
   * Must be reimplemented in derived classes.
   */
  virtual PropertyValue *init (int size) = 0;

  virtual int size () = 0;

  /**
   * Resizes the property to the size n
   * Must be reimplemented in derived classes.
   */
  virtual void resize (int n) = 0;

  virtual void shallowCopy (PropertyValue *rhs) = 0;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class MaterialProperty : public PropertyValue
{
public:
  virtual ~MaterialProperty()
  {
    _value.release();
  }

  /**
   * @returns a read-only reference to the parameter value.
   */
  MooseArray<T> & get () { return _value; }

  /**
   * @returns a writeable reference to the parameter value.
   */
  MooseArray<T> & set () { return _value; }

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type ();

  /**
   * Clone this value.  Useful in copy-construction.
   */
  virtual PropertyValue *init (int size);

  /**
   * Resizes the property to the size n
   */
  virtual void resize (int n);

  /**
   * Get element i out of the array.
   */
  T & operator[](const unsigned int i) { return _value[i]; }

  int size() { return _value.size(); }

  /**
   * Get element i out of the array.
   */
  const T & operator[](const unsigned int i) const { return _value[i]; }

  /**
   *
   */
  virtual void shallowCopy (PropertyValue *rhs);

private:

  /// Stored parameter value.
  MooseArray<T> _value;
};


// ------------------------------------------------------------
// Material::Property<> class inline methods
template <typename T>
inline std::string
MaterialProperty<T>::type ()
{
  return typeid(T).name();
}

template <typename T>
inline PropertyValue *
MaterialProperty<T>::init (int size)
{
  MaterialProperty<T> *copy = new MaterialProperty<T>;
  copy->_value.resize(size, 0);
  return copy;
}

/**
 * Specialization of init function for std::vector<Real>
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<std::vector<Real> >::init (int size);

/**
 * Specialization of init function for ColumnMajorMatrix
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<ColumnMajorMatrix>::init (int size);

/**
 * Specialization of init function for std::vector<ColumnMajorMatrix>
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<std::vector<ColumnMajorMatrix> >::init (int size);

/**
 * Specialization of init function for std::vector<RealTensorValue>
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<std::vector<RealTensorValue> >::init (int size);

/**
 * Specialization of init function for std::vector<std::vector<RealTensorValue> >
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<std::vector<std::vector<RealTensorValue> > >::init (int size);

/**
 * Specialization of init function for std::vector<RealVectorValue>
 * See MaterialPropertyInterface.C for implementation.
 */
template <>
PropertyValue *
MaterialProperty<std::vector<RealVectorValue> >::init (int size);



template <typename T>
inline void
MaterialProperty<T>::resize (int n)
{
  _value.resize(n);
}

template <typename T>
inline void
MaterialProperty<T>::shallowCopy (PropertyValue *rhs)
{
  mooseAssert(rhs != NULL, "Assigning NULL?");
  _value.shallowCopy(libmesh_cast_ptr<const MaterialProperty<T>*>(rhs)->_value);
}


/**
 * Container for storing material properties
 */
class MaterialProperties : public std::vector<PropertyValue *>
{
public:
  MaterialProperties() { }

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
};

#endif
