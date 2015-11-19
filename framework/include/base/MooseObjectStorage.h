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

#ifndef MOOSEOBJECTSTORAGE_H
#define MOOSEOBJECTSTORAGE_H

// MOOSE includes
#include "ObjectStorage.h"
#include "DependencyResolverInterface.h"
#include "BlockRestrictable.h"

/**
 * A storage container for MooseObjects.
 *
 * This container is stores threaded copies and includes automatic storage of block/boundary
 * restricted objects. It also, maintains lists of active objects for use by Controls.
 */
template<typename MooseObjectType>
class MooseObjectStorage : public ObjectStorage<MooseObjectType>
{
public:

  // Use the following from the base class
  using ObjectStorage<MooseObjectType>::_num_threads;
  using ObjectStorage<MooseObjectType>::_all_objects;

   /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectStorage(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~MooseObjectStorage();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  virtual void addObject(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Provides [] operator access to the threaded storage.
   */
  inline std::vector<MooseSharedPointer<MooseObjectType> > & operator[](THREAD_ID tid) { return _active_objects[tid]; }
  inline const std::vector<MooseSharedPointer<MooseObjectType> > & operator[](THREAD_ID tid) const { return _active_objects[tid]; }
  //@}

  ///@{
  /**
   * Retrieve complete storage to the block/boundary restricted objects for a given thread.
   * @param tid The thread id to retrieve objects from
   */
  inline const std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > > & getActiveBlockObjects(THREAD_ID tid = 0) const { return _active_block_objects[tid]; }
  inline const std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > > & getActiveBoundaryObjects(THREAD_ID tid = 0) const { return _active_boundary_objects[tid]; }
  ///*}

  ///@{
  /**
   * Convenience functions for determining if block/boundary restricted objects exist.
   */
  bool hasActiveBlockObjects(THREAD_ID tid = 0) const;
  bool hasActiveBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Updates the active objects storage.
   */
  virtual void updateActive(THREAD_ID tid = 0);

  /**
   * Return the number of active objects stored in this container.
   */
  inline unsigned int size(THREAD_ID tid = 0) const { return _active_objects[tid].size(); }

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   */
  void initialSetup(THREAD_ID tid = 0) const;
  void timestepSetup(THREAD_ID tid = 0) const;
  void subdomainSetup(THREAD_ID tid = 0) const;
  void jacobianSetup(THREAD_ID tid = 0) const;
  void residualSetup(THREAD_ID tid = 0) const;
  ///@}
  /**
   * Sort the objects using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

protected:

  // All block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > > > _all_block_objects;

  // All boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > > >_all_boundary_objects;

  /// All active objects, this is the counterpart to ObjectStorage::_all_objects (THREAD_ID on outer vector)
  std::vector<std::vector<MooseSharedPointer<MooseObjectType> > >_active_objects;

  /// Active block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > > > _active_block_objects;

  /// Active boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > > >_active_boundary_objects;

  /**
   * Helper method for updating active vectors
   */
  static void updateActiveHelper(std::vector<MooseSharedPointer<MooseObjectType> > & active, const std::vector<MooseSharedPointer<MooseObjectType> > & all);

  /**
   * Helper method for sorting vectors of objects.
   */
  static void sortHelper(std::vector<MooseSharedPointer<MooseObjectType> > & objects);

  /**
   * Calls assert on thread id.
   */
  void checkThreadID(THREAD_ID tid) const;

};


template<typename MooseObjectType>
MooseObjectStorage<MooseObjectType>::MooseObjectStorage(bool threaded /*=true*/) :
    ObjectStorage<MooseObjectType>(threaded),
    _all_block_objects(_num_threads),
    _all_boundary_objects(_num_threads),
    _active_objects(_num_threads),
    _active_block_objects(_num_threads),
    _active_boundary_objects(_num_threads)
{
}


template<typename MooseObjectType>
MooseObjectStorage<MooseObjectType>::~MooseObjectStorage()
{
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::addObject(MooseSharedPointer<MooseObjectType> object, THREAD_ID tid /*= 0*/)
{
  checkThreadID(tid);

  // Stores object in list of all objects
  ObjectStorage<MooseObjectType>::addObject(object);

  // If enabled, store object in a list of all active
  bool enabled = object->enabled();
  if (enabled)
    _active_objects[tid].push_back(object);

  // Boundary Restricted
  if (object->boundaryRestricted())
  {
    for (std::set<BoundaryID>::const_iterator it = object->boundaryIDs().begin(); it != object->boundaryIDs().end(); ++it)
    {
      _all_boundary_objects[tid][*it].push_back(object);
      if (enabled)
        _active_boundary_objects[tid][*it].push_back(object);
    }
  }

  // Block Restricted
  else if (object->isBlockRestrictable())
  {
    const std::set<SubdomainID> & ids = object->blockIDs(/*mesh_ids=*/true);
    for (std::set<SubdomainID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
      _all_block_objects[tid][*it].push_back(object);
      if (enabled)
        _active_block_objects[tid][*it].push_back(object);
    }
  }
}


template<typename MooseObjectType>
bool
MooseObjectStorage<MooseObjectType>::hasActiveBlockObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_block_objects = false;
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator it;
  for (it = _active_block_objects[tid].begin(); it != _active_block_objects[tid].end(); ++it)
    has_active_block_objects |= !(it->second.empty());
  return has_active_block_objects;
}


template<typename MooseObjectType>
bool
MooseObjectStorage<MooseObjectType>::hasActiveBlockObjects(SubdomainID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator iter = _active_block_objects[tid].find(id);
  if (iter != _active_block_objects[tid].end() && !iter->second.empty())
    return true;
  return false;
}


template<typename MooseObjectType>
bool
MooseObjectStorage<MooseObjectType>::hasActiveBoundaryObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_boundary_objects = false;
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator it;
  for (it = _active_boundary_objects[tid].begin(); it != _active_boundary_objects[tid].end(); ++it)
    has_active_boundary_objects |= !(it->second.empty());
  return has_active_boundary_objects;
}


template<typename MooseObjectType>
bool
MooseObjectStorage<MooseObjectType>::hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator iter = _active_boundary_objects[tid].find(id);
  if (iter != _active_boundary_objects[tid].end() && !iter->second.empty())
    return true;
  return false;
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::updateActive(THREAD_ID tid /*= 0*/)
{
  checkThreadID(tid);

  updateActiveHelper(_active_objects[tid], _all_objects[tid]);

  {
    typename std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator it;
    for (it = _all_block_objects[tid].begin(); it != _all_block_objects[tid].end(); ++it)
      updateActiveHelper(_active_block_objects[tid][it->first], it->second);
  }


  {
    typename std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > >::const_iterator it;
    for (it = _all_boundary_objects[tid].begin(); it != _all_boundary_objects[tid].end(); ++it)
      updateActiveHelper(_active_boundary_objects[tid][it->first], it->second);
  }

}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::updateActiveHelper(std::vector<MooseSharedPointer<MooseObjectType> > & active, const std::vector<MooseSharedPointer<MooseObjectType> > & all)
{
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator iter;

  // Clear the active list
  active.clear();

  // Add "enabled" objects to the active list
  for (iter = all.begin(); iter != all.end(); ++iter)
    if ( (*iter)->enabled() )
      active.push_back(*iter);
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::initialSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->initialSetup();
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::timestepSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->timestepSetup();
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::subdomainSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->subdomainSetup();
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::jacobianSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->jacobianSetup();
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::residualSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<MooseObjectType> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->residualSetup();
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::sort(THREAD_ID tid/* = 0*/)
{
  checkThreadID(tid);

  {
    typename std::map<SubdomainID, std::vector<MooseSharedPointer<MooseObjectType> > >::iterator iter;
    for (iter = _all_block_objects[tid].begin(); iter != _all_block_objects[tid].end(); ++iter)
      sortHelper(iter->second);
  }

  {
    typename std::map<BoundaryID, std::vector<MooseSharedPointer<MooseObjectType> > >::iterator iter;
    for (iter = _all_boundary_objects[tid].begin(); iter != _all_boundary_objects[tid].end(); ++iter)
      sortHelper(iter->second);
  }

  sortHelper(_all_objects[tid]);

  // The active lists now must be update to reflect the order changes
  updateActive(tid);
}


template<typename MooseObjectType>
void
MooseObjectStorage<MooseObjectType>::sortHelper(std::vector<MooseSharedPointer<MooseObjectType> > & objects)
{
  // Do nothing if the vector is empty
  if (objects.empty())
    return;

  // Make sure the object is sortable
  mooseAssert(MooseSharedNamespace::dynamic_pointer_cast<DependencyResolverInterface>(objects[0]), "Objects must inhert from DependencyResolverInterface to be sorted.");

  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort<MooseSharedPointer<MooseObjectType> >(objects);
  }
  catch(CyclicDependencyException<MooseSharedPointer<MooseObjectType> > & e)
  {
    DependencyResolverInterface::cyclicDependencyError<MooseSharedPointer<MooseObjectType> >(e, "Cyclic dependency detected in object ordering");
  }
}

template<typename MooseObjectType>
inline void
MooseObjectStorage<MooseObjectType>::checkThreadID(THREAD_ID tid) const
{
  mooseAssert(tid < _num_threads, "Attempting to access a thread id (" << tid << ") greater than the number allowed by the storage item (" << _num_threads << ")");
}

#endif // OBJECTSTORAGE_H
