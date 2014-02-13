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

#ifndef RESTARTABLEDATA_H
#define RESTARTABLEDATA_H

#include <vector>

#include "ColumnMajorMatrix.h"
#include "MaterialPropertyIO.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

class RestartableDataValue;

/**
 * Abstract definition of a RestartableData value.
 */
class RestartableDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   * @param context 'typeless' pointer to user-specific data.
   */
  RestartableDataValue(std::string name, void * context) :
      _name(name),
      _context(context)
    {}

  /**
   * Destructor.
   */
  virtual ~RestartableDataValue() {};

  /**
   * String identifying the type of parameter stored.
   * Must be reimplemented in derived classes.
   */
  virtual std::string type () = 0;

  /**
   * The full (unique) name of this particular piece of data.
   */
  std::string name() { return _name; }

  /**
   * A context pointer for helping with load / store.
   */
  void * context() { return _context; }

  virtual void swap (RestartableDataValue *rhs) = 0;

  // save/restore in a file
  virtual void store(std::ostream & stream) = 0;
  virtual void load(std::istream & stream) = 0;

protected:
  /// The full (unique) name of this particular piece of data.
  std::string _name;

  /// A context pointer for helping with load and store
  void * _context;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class RestartableData : public RestartableDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   * @param context 'typeless' pointer to user-specific data.
   */
  RestartableData(std::string name, void * context) :
      RestartableDataValue(name, context)
  {
    _value_ptr = new T;
  }

  virtual ~RestartableData() { delete _value_ptr; }

  /**
   * @returns a read-only reference to the parameter value.
   */
  T & get () { return *_value_ptr; }

  /**
   * @returns a writable reference to the parameter value.
   */
  T & set () { return *_value_ptr; }

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type ();

  /**
   * Swap
   */
  virtual void swap (RestartableDataValue *rhs);

  /**
   * Store the RestartableData into a binary stream
   */
  virtual void store(std::ostream & stream);

  /**
   * Load the RestartableData from a binary stream
   */
  virtual void load(std::istream & stream);

private:

  /// Stored value.
  T * _value_ptr;
};


// ------------------------------------------------------------
// RestartableData<> class inline methods
template <typename T>
inline std::string
RestartableData<T>::type ()
{
  return typeid(T).name();
}

template <typename T>
inline void
RestartableData<T>::swap (RestartableDataValue *rhs)
{
  mooseAssert(rhs != NULL, "Assigning NULL?");
//  _value.swap(libmesh_cast_ptr<RestartableData<T>*>(rhs)->_value);
}

template<typename T>
inline void
RestartableData<T>::store(std::ostream & stream)
{
  T & tmp = *_value_ptr;
  storeHelper(stream, tmp, _context);
}

template<typename T>
inline void
RestartableData<T>::load(std::istream & stream)
{
  loadHelper(stream, *_value_ptr, _context);
}

/**
 * Container for storing material properties
 */
class RestartableDatas : public std::vector<std::map<std::string, RestartableDataValue *> >
{
public:
  RestartableDatas(size_type n) :
      std::vector<std::map<std::string, RestartableDataValue *> >(n)
    {
    }


  virtual ~RestartableDatas()
    {
      for (std::vector<std::map<std::string, RestartableDataValue *> >::iterator i = begin(); i != end(); ++i)
        for (std::map<std::string, RestartableDataValue *>::iterator j=(*i).begin();
             j != (*i).end();
             ++j)
          delete j->second;
    }

//   /**
//    * Parameter map iterator.
//    */
//   typedef std::vector<RestartableDataValue *>::iterator iterator;

//   /**
//    * Constant parameter map iterator.
//    */
//   typedef std::vector<RestartableDataValue *>::const_iterator const_iterator;
};
#endif
