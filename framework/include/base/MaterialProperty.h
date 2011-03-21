#ifndef MATERIALPROPERTY_H_
#define MATERIALPROPERTY_H_

#include <vector>

#include "Array.h"

#include "libmesh_common.h"

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
  Array<T> & get () { return _value; }

  /**
   * @returns a writeable reference to the parameter value.
   */
  Array<T> & set () { return _value; }

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
  Array<T> _value;
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



/**
 * Container for storing material properties
 */
class MaterialProperties : public std::map<std::string, PropertyValue *>
{
public:
  MaterialProperties() { }

  /**
   * Parameter map iterator.
   */
  typedef std::map<std::string, PropertyValue *>::iterator iterator;

  /**
   * Constant parameter map iterator.
   */
  typedef std::map<std::string, PropertyValue *>::const_iterator const_iterator;
};

#endif
