/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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

#include "MooseArray.h"


/**
 * Abstract definition of a property value.
 */
class PropertyValue : public ReferenceCountedObject<PropertyValue>
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
  virtual PropertyValue *init () = 0;

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
  virtual PropertyValue *init ();

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

  /**
   * Stored parameter value.
   */
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
MaterialProperty<T>::init ()
{
  MaterialProperty<T> *copy = new MaterialProperty<T>;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size(), 0);

  return copy;
}

template <>
PropertyValue *
MaterialProperty<std::vector<Real> >::init ();

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
  if (rhs == NULL)
    _value.resize(0);
  else
    _value.shallowCopy(dynamic_cast<const MaterialProperty<T>*>(rhs)->_value);
}

#endif
