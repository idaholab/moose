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
template<typename T>
class ExecuteMooseObjectStorage : public MooseObjectStorage<T>
{
public:

  using MooseObjectStorage<T>::checkThreadID;
  using MooseObjectStorage<T>::initialSetup;
  using MooseObjectStorage<T>::timestepSetup;
  using MooseObjectStorage<T>::subdomainSetup;

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
  virtual void addObject(MooseSharedPointer<T> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Retrieve shared pointers for the given thread and execution type for all/active objects.
   * @param exec_flag The execution flag to retrieve objects from
   */
  const MooseObjectStorage<T> & operator[](ExecFlagType exec_flag) const;
  MooseObjectStorage<T> & operator[](ExecFlagType exec_flag);
  ///@}

  ///@{
  /**
   * Provide access to begin/end iterators of the underlying map of execution flags.
   */
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::const_iterator begin() const { return _execute_objects.begin(); }
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::const_iterator end() const { return _execute_objects.end(); }
  ///@}

  /**
   * Updates the active objects storage.
   */
  virtual void updateActive(THREAD_ID tid = 0);

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   *
   * Limits call to these methods only to objects being executed on linear/nonlinear iterations.
   */
  void jacobianSetup(THREAD_ID tid = 0) const;
  void residualSetup(THREAD_ID tid = 0) const;
  void setup(ExecFlagType exec_flag, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);


protected:

  // Map of execute objects to storage containers for MooseObjects
  std::map<ExecFlagType, MooseObjectStorage<T> > _execute_objects;

  /// A helper method for extracting objects from the various storage containers
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::iterator
  getStorageHelper(std::map<ExecFlagType, MooseObjectStorage<T> > & objects, ExecFlagType exec_flag) const;
};


template<typename T>
ExecuteMooseObjectStorage<T>::ExecuteMooseObjectStorage(bool threaded) :
    MooseObjectStorage<T>(threaded)
{
  // Initialize the active/all data structures with the correct map entries and empty vectors
  for (std::vector<ExecFlagType>::const_iterator it = Moose::exec_types.begin(); it != Moose::exec_types.end(); ++it)
  {
    std::pair<ExecFlagType, MooseObjectStorage<T> > item(*it, MooseObjectStorage<T>(threaded));
    _execute_objects.insert(item);
  }
}


template<typename T>
ExecuteMooseObjectStorage<T>::~ExecuteMooseObjectStorage()
{
}


template<typename T>
const MooseObjectStorage<T> &
ExecuteMooseObjectStorage<T>::operator[](ExecFlagType exec_flag) const
{
  // Use find to avoid accidental insertion
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::const_iterator iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the global list of execute parameters is likely out-of-date.");

  return iter->second;
}

template<typename T>
MooseObjectStorage<T> &
ExecuteMooseObjectStorage<T>::operator[](ExecFlagType exec_flag)
{
  // Use find to avoid accidental insertion
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::iterator iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the global list of execute parameters is likely out-of-date.");

  return iter->second;
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::updateActive(THREAD_ID tid/* = 0 */)
{
  // Update all objects active list
  MooseObjectStorage<T>::updateActive(tid);

  // Update the execute flag lists of objects
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::iterator iter;
  for (iter = _execute_objects.begin(); iter != _execute_objects.end(); ++iter)
    iter->second.updateActive(tid);
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::jacobianSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::const_iterator iter = _execute_objects.find(EXEC_NONLINEAR);
  if (iter != _execute_objects.end())
    iter->second.jacobianSetup(tid);
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::residualSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::const_iterator iter = _execute_objects.find(EXEC_LINEAR);
  if (iter != _execute_objects.end())
    iter->second.residualSetup(tid);
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::setup(ExecFlagType exec_flag, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  switch (exec_flag)
  {
  case EXEC_INITIAL:
    initialSetup(tid);
    break;
  case EXEC_TIMESTEP_BEGIN:
    timestepSetup(tid);
    break;
  case EXEC_SUBDOMAIN:
    subdomainSetup(tid);
    break;
  case EXEC_NONLINEAR:
    jacobianSetup(tid);
    break;
  case EXEC_LINEAR:
    residualSetup(tid);
    break;
  default:
    break;
  }
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::addObject(MooseSharedPointer<T> object, THREAD_ID tid/*=0*/)
{
  // Update list of all objects
  MooseObjectStorage<T>::addObject(object, tid);

  // Update the execute flag lists of objects
  MooseSharedPointer<SetupInterface> ptr = MooseSharedNamespace::dynamic_pointer_cast<SetupInterface>(object);
  if (ptr)
  {
    const std::vector<ExecFlagType> flags = ptr->execFlags();
    for (std::vector<ExecFlagType>::const_iterator it = flags.begin(); it != flags.end(); ++it)
      _execute_objects[*it].addObject(object, tid);
  }
}


template<typename T>
void
ExecuteMooseObjectStorage<T>::sort(THREAD_ID tid/* = 0*/)
{
  // Sort all objects
  MooseObjectStorage<T>::sort(tid);

  // Sort execute object storage
  typename std::map<ExecFlagType, MooseObjectStorage<T> >::iterator iter;
  for (iter = _execute_objects.begin(); iter != _execute_objects.end(); ++iter)
    iter->second.sort(tid);
}

#endif // EXECUTEMOOSEOBJECTSTORAGE_H
