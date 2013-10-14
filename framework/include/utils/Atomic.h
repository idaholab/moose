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

#ifndef MOOSEATOMIC_H
#define MOOSEATOMIC_H

// Libmesh threading
#include "libmesh/threads.h"

/**
 * A thread-safe Atomic object that mimics the behavior of tbb::atomic. This was developed
 * to avoid problems that occured with inhertence between libMesh::Threads::atomic and tbb::atomic
 * that were occuring. Additionally, this allowed for the addition of a value based constructor
 * that is not available via libMesh::Threads::atomic
 */
template <typename T>
class MooseAtomic
{
public:

  /**
   * Default constructor; initializes the stored value to 0
   */
  MooseAtomic () : _value(0){}

  /**
   * Value based constructor; initializes to the supplied value
   * @param value The initial value
   */
  MooseAtomic(T value) : _value(value){}

  /**
   * Return the stored value
   * @return Returns stored value of type T
   */
  operator T () const {return _value;}

  /**
   * Assignment operator
   * @param value The value to store
   */
  T operator=( T value )
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value = value;
    return _value;
  }

  /**
   * Assignment operator (MooseAtomic input)
   * @param value Assigns the value from another MooseAtomic object to this object
   */
  MooseAtomic<T>& operator=( const MooseAtomic<T>& value )
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value = value;
    return *this;
  }

  /**
   * Addition operator
   * @param value Adds this amount to the stored value
   */
  T operator+=(T value)
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value += value;
    return _value;
  }

  /**
   * Substraction operator
   * @param value Subtracts this amount to the stored value
   */
  T operator-=(T value)
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value -= value;
    return _value;
  }

  /**
   * Addition increment operator
   * adds 1.0 to the stored value
   */
  T operator++()
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value++;
    return _value;
  }

  /**
   * Subtraction increment operator
   * subtracts 1.0 to the stored value
   */
  T operator--()
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(_smutex);
    _value--;
    return _value;
  }

protected:
  /// The stored value
  T _value;

  /// spin_mutex object for applying scoped_lock to _value access
  libMesh::Threads::spin_mutex _smutex;
};









#endif // MOOSEATOMIC_H
