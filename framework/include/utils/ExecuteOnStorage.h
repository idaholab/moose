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

#ifndef EXECUTEONSTORAGE_H
#define EXECUTEONSTORAGE_H

// MOOSE includes
#include "MooseTypes.h"
#include "SetupInterface.h"

/**
 * A class for storing objects based on execution flag.
 *
 * @see WarehouseBase
 */
template<typename MooseObjectType>
class ExecuteOnStorage
{
public:

  /**
   * Constructor.
   * @param num_threads Set to libMesh::n_threads() to store threaded copies.
   */
  ExecuteOnStorage(THREAD_ID num_threads = 1);

  /**
   * Destructor.
   */
  virtual ~ExecuteOnStorage();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   * @param tid The thread of the object
   */
  void add(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid = 0);

  /**
   * Retrieve shared pointers for the given thread and execution type.
   * @param exec_flag The execution flag to retrieve objects from
   * @param tid The thread id to retrieve objects from
   */
  const std::vector<MooseSharedPointer<MooseObjectType> > & get(const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<MooseObjectType> > & getActive(const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;

  /**
   * Updates the active objects storage.
   */
  void updateActive();

  /// Return the size of the threaded vector
  unsigned int size();

private:

  /// Storage container for the ALL pointers
  std::vector<std::map<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > > > _all_objects;

  /// Storage container for the ACTIVE pointers
  std::vector<std::map<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > > > _active_objects;

  /// A helper method for extarcting objects from the various storage containers
  const std::vector<MooseSharedPointer<MooseObjectType> > & getObjectsHelper(const std::map<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > > & objects,
                                                                             const ExecFlagType & exec_flag) const;

};


template<typename MooseObjectType>
const std::vector<MooseSharedPointer<MooseObjectType> > &
ExecuteOnStorage<MooseObjectType>::get(const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  return getObjectsHelper(_all_objects[tid], exec_flag);
}


template<typename MooseObjectType>
const std::vector<MooseSharedPointer<MooseObjectType> > &
ExecuteOnStorage<MooseObjectType>::getActive(const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  return getObjectsHelper(_active_objects[tid], exec_flag);
}


template<typename MooseObjectType>
const std::vector<MooseSharedPointer<MooseObjectType> > &
ExecuteOnStorage<MooseObjectType>::getObjectsHelper(const std::map<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > > & objects,
                                                    const ExecFlagType & exec_flag) const
{
  // Use find to avoid accidental insertion
  typename std::map<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator iter = objects.find(exec_flag);

  if (iter == objects.end())
    mooseError("Unable to locate the desired execute flag, the global list of execute parameters is likely out-of-date.");
  return (iter->second);
}


template<typename MooseObjectType>
ExecuteOnStorage<MooseObjectType>::~ExecuteOnStorage()
{
}


template<typename MooseObjectType>
ExecuteOnStorage<MooseObjectType>::ExecuteOnStorage(THREAD_ID num_threads) :
    _all_objects(num_threads),
    _active_objects(num_threads)
{
  // Initialize the active/all data structures with the correct map entries and empty vectors
  for (unsigned int tid = 0; tid < num_threads; ++tid)
    for (std::vector<ExecFlagType>::const_iterator it = Moose::exec_types.begin(); it != Moose::exec_types.end(); ++it)
    {
      std::pair<ExecFlagType, std::vector<MooseSharedPointer<MooseObjectType> > > item(*it, std::vector<MooseSharedPointer<MooseObjectType> >());
      _all_objects[tid].insert(item);
      _active_objects[tid].insert(item);
    }
}


template<typename MooseObjectType>
void
ExecuteOnStorage<MooseObjectType>::updateActive()
{

  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator all_iter;

  for (unsigned int tid = 0; tid < size(); ++tid)
  {
    for (std::vector<ExecFlagType>::const_iterator flag_iter = Moose::exec_types.begin(); flag_iter != Moose::exec_types.end(); ++flag_iter)
    {
      // Clear the existing active vector
      _active_objects[tid][*flag_iter].clear();

      // Add "enabled" objects to the active list
      for (all_iter = _all_objects[tid][*flag_iter].begin(); all_iter != _all_objects[tid][*flag_iter].end(); ++all_iter)
        if ( (*all_iter)->enabled() )
          _active_objects[tid][*flag_iter].push_back(*all_iter);
    }
  }
}


template<typename MooseObjectType>
void
ExecuteOnStorage<MooseObjectType>::add(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid)
{
  MooseSharedPointer<SetupInterface> ptr = MooseSharedNamespace::dynamic_pointer_cast<SetupInterface>(object);
  if (ptr)
  {
    const std::vector<ExecFlagType> flags = ptr->execFlags();
    for (std::vector<ExecFlagType>::const_iterator it = flags.begin(); it != flags.end(); ++it)
    {
      _all_objects[tid][*it].push_back(object);

      if (object->enabled())
        _active_objects[tid][*it].push_back(object);
    }
  }
}


template<typename MooseObjectType>
unsigned int
ExecuteOnStorage<MooseObjectType>::size()
{
  return _all_objects.size();
}

#endif // EXECUTEONSTORAGE_H
