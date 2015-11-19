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

#ifndef OBJECTSTORAGE_H
#define OBJECTSTORAGE_H

// MOOSE includes
#include "MooseTypes.h"

/**
 * A general storage container for objects capable of storing threaded copies.
 */
template<typename ObjectType>
class ObjectStorage
{
public:

  /**
   * Constructor.
   */
  ObjectStorage(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~ObjectStorage();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  virtual void addObject(MooseSharedPointer<ObjectType> object, THREAD_ID tid = 0);

  /**
   * Retrieve vector of shared pointers for the given thread.
   */
  const std::vector<MooseSharedPointer<ObjectType> > & getObjects(THREAD_ID tid = 0) const;

  /// Return the size of the vector for a given thread.
  virtual unsigned int size(THREAD_ID tid = 0) const;

  /// Perform thread it error check.
//  void checkThreadID(THREAD_ID tid = 0);



protected:

  /// Convenience member storing the number of threads used for storage (1 or libMesh::n_threads)
  const THREAD_ID _num_threads;

  /// Storage container for the ALL pointers
  std::vector<std::vector<MooseSharedPointer<ObjectType> > > _all_objects;

};


template<typename ObjectType>
ObjectStorage<ObjectType>::ObjectStorage(bool threaded /*=true*/) :
    _num_threads(threaded ? libMesh::n_threads() : 1),
    _all_objects(_num_threads)
{
}


template<typename ObjectType>
ObjectStorage<ObjectType>::~ObjectStorage()
{
}


template<typename ObjectType>
void
ObjectStorage<ObjectType>::addObject(MooseSharedPointer<ObjectType> object, THREAD_ID tid /*= 0*/)
{
  _all_objects[tid].push_back(object);
}


template<typename ObjectType>
const std::vector<MooseSharedPointer<ObjectType> > &
ObjectStorage<ObjectType>::getObjects(THREAD_ID tid /*= 0*/) const
{
  return _all_objects[tid];
}


template<typename ObjectType>
unsigned int
ObjectStorage<ObjectType>::size(THREAD_ID tid /*= 0*/) const
{
  return _all_objects[tid].size();
}

#endif // OBJECTSTORAGE_H
