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

#ifndef EXECUTEMOOSEOBJECTSTORAGE_H
#define EXECUTEMOOSEOBJECTSTORAGE_H

// MOOSE includes
#include "MooseObjectStorage.h"
#include "SetupInterface.h"

/**
 * A class for storing MooseObjects based on execution flag.
 */
template<typename MooseObjectType>
class ExecuteMooseObjectStorage
{
public:

  /**
   * Constructor.
   * @param threaded True enables threaded object storage (default).
   */
  ExecuteMooseObjectStorage(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~ExecuteMooseObjectStorage();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  virtual void addObject(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Retrieve shared pointers for the given thread and execution type for all/active objects.
   * @param exec_flag The execution flag to retrieve objects from
   */
  const MooseObjectStorage<MooseObjectType> & operator[](ExecFlagType exec_flag) const;
  MooseObjectStorage<MooseObjectType> & operator[](ExecFlagType exec_flag);
  ///@}

  ///@{
  /**
   * Provide access to begin/end iterators of the underlying map of execution flags.
   */
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator begin(){ return _execute_objects.begin(); }
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator end(){ return _execute_objects.end(); }
  ///@}

  /**
   * Updates the active objects storage.
   */
  virtual void updateActive(THREAD_ID tid = 0);

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);


protected:

  // Map of execute objects to storage containers for MooseObjects
  std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> > _execute_objects;

  /// A helper method for extracting objects from the various storage containers
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator
  getStorageHelper(std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> > & objects, ExecFlagType exec_flag) const;
};


template<typename MooseObjectType>
ExecuteMooseObjectStorage<MooseObjectType>::ExecuteMooseObjectStorage(bool threaded)
{
  // Initialize the active/all data structures with the correct map entries and empty vectors
  for (std::vector<ExecFlagType>::const_iterator it = Moose::exec_types.begin(); it != Moose::exec_types.end(); ++it)
  {
    std::pair<ExecFlagType, MooseObjectStorage<MooseObjectType> > item(*it, MooseObjectStorage<MooseObjectType>(threaded));
    _execute_objects.insert(item);
  }
}


template<typename MooseObjectType>
ExecuteMooseObjectStorage<MooseObjectType>::~ExecuteMooseObjectStorage()
{
}


template<typename MooseObjectType>
const MooseObjectStorage<MooseObjectType> &
ExecuteMooseObjectStorage<MooseObjectType>::operator[](ExecFlagType exec_flag) const
{
  // Use find to avoid accidental insertion
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::const_iterator iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the global list of execute parameters is likely out-of-date.");

  return iter->second;
}

template<typename MooseObjectType>
MooseObjectStorage<MooseObjectType> &
ExecuteMooseObjectStorage<MooseObjectType>::operator[](ExecFlagType exec_flag)
{
  // Use find to avoid accidental insertion
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the global list of execute parameters is likely out-of-date.");

  return iter->second;
}


template<typename MooseObjectType>
void
ExecuteMooseObjectStorage<MooseObjectType>::updateActive(THREAD_ID tid/* = 0 */)
{
  // Update the execute flag lists of objects
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator iter;
  for (iter = _execute_objects.begin(); iter != _execute_objects.end(); ++iter)
    iter->second.updateActive(tid);
}


template<typename MooseObjectType>
void
ExecuteMooseObjectStorage<MooseObjectType>::addObject(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid/*=0*/)
{
  // Update the execute flag lists of objects
  MooseSharedPointer<SetupInterface> ptr = MooseSharedNamespace::dynamic_pointer_cast<SetupInterface>(object);
  if (ptr)
  {
    const std::vector<ExecFlagType> flags = ptr->execFlags();
    for (std::vector<ExecFlagType>::const_iterator it = flags.begin(); it != flags.end(); ++it)
      _execute_objects[*it].addObject(object, tid);
  }
}


template<typename MooseObjectType>
void
ExecuteMooseObjectStorage<MooseObjectType>::sort(THREAD_ID tid/* = 0*/)
{
  // Sort execute object storage
  typename std::map<ExecFlagType, MooseObjectStorage<MooseObjectType> >::iterator iter;
  for (iter = _execute_objects.begin(); iter != _execute_objects.end(); ++iter)
    iter->second.sort(tid);
}

#endif // EXECUTEMOOSEOBJECTSTORAGE_H
