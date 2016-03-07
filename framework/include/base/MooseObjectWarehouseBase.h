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

#ifndef MOOSEOBJECTWAREHOUSEBASE_H
#define MOOSEOBJECTWAREHOUSEBASE_H

// MOOSE includes
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "BlockRestrictable.h"
#include "MooseVariable.h"
#include "TransientInterface.h"

/**
 * A base storage container for MooseObjects.
 */
template<typename T>
class MooseObjectWarehouseBase
{
public:

  /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectWarehouseBase(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~MooseObjectWarehouseBase();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  virtual void addObject(MooseSharedPointer<T> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Retrieve complete vector to the all/block/boundary restricted objects for a given thread.
   * @param tid The thread id to retrieve objects from
   */
  const std::vector<MooseSharedPointer<T> > & getObjects(THREAD_ID tid = 0) const;
  const std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > & getBlockObjects(THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<T> > & getBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  const std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > & getBoundaryObjects(THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<T> > & getBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Retrieve complete vector to the active all/block/boundary restricted objects for a given thread.
   * @param tid The thread id to retrieve objects from
   */
  const std::vector<MooseSharedPointer<T> > & getActiveObjects(THREAD_ID tid = 0) const;
  const std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > & getActiveBlockObjects(THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<T> > & getActiveBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  const std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > & getActiveBoundaryObjects(THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<T> > & getActiveBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Convenience functions for determining if objects exist.
   */
  bool hasActiveObjects(THREAD_ID tid = 0) const;
  bool hasActiveBlockObjects(THREAD_ID tid = 0) const;
  bool hasActiveBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Return a set of active SubdomainsIDs
   */
  std::set<SubdomainID> getActiveBlocks(THREAD_ID tid = 0) const;

  ///@{
  /**
   * Convenience functions for checking/getting specific objects
   */
  bool hasActiveObject(const std::string & name, THREAD_ID tid = 0) const;
  MooseSharedPointer<T> getActiveObject(const std::string & name, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Updates the active objects storage.
   */
  virtual void updateActive(THREAD_ID tid = 0);

  /**
   * Sort the objects using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

  ///@{
  /**
   * Update variable dependency vector.
   */
  void updateVariableDependency(std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid = 0) const;
  void updateBlockVariableDependency(SubdomainID id, std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid = 0) const;
  void updateBoundaryVariableDependency(std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid = 0) const;
  void updateBoundaryVariableDependency(BoundaryID id, std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Populates a set of covered subdomains and the associated variable names.
   */
  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables, THREAD_ID tid = 0) const;


protected:

  /// Convenience member storing the number of threads used for storage (1 or libMesh::n_threads)
  const THREAD_ID _num_threads;

  /// Storage container for the ALL pointers (THREAD_ID on outer vector)
  std::vector<std::vector<MooseSharedPointer<T> > > _all_objects;

  /// All active objects (THREAD_ID on outer vector)
  std::vector<std::vector<MooseSharedPointer<T> > > _active_objects;

  // All block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > > _all_block_objects;

  /// Active block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > > _active_block_objects;

  // All boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > > _all_boundary_objects;

  /// Active boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > > _active_boundary_objects;

  /**
   * Helper method for updating active vectors
   */
  static void updateActiveHelper(std::vector<MooseSharedPointer<T> > & active, const std::vector<MooseSharedPointer<T> > & all);

  /**
   * Helper method for sorting vectors of objects.
   */
  static void sortHelper(std::vector<MooseSharedPointer<T> > & objects);

  /**
   * Helper method for updating variable dependency vector
   */
  static void updateVariableDependencyHelper(std::set<MooseVariable *> & needed_moose_vars,
                                             const std::vector<MooseSharedPointer<T> > & objects);

  /**
   * Calls assert on thread id.
   */
  void checkThreadID(THREAD_ID tid) const;

};


template<typename T>
MooseObjectWarehouseBase<T>::MooseObjectWarehouseBase(bool threaded /*=true*/) :
    _num_threads(threaded ? libMesh::n_threads() : 1),
    _all_objects(_num_threads),
    _active_objects(_num_threads),
    _all_block_objects(_num_threads),
    _active_block_objects(_num_threads),
    _all_boundary_objects(_num_threads),
    _active_boundary_objects(_num_threads)
{
}


template<typename T>
MooseObjectWarehouseBase<T>::~MooseObjectWarehouseBase()
{
}


template<typename T>
void
MooseObjectWarehouseBase<T>::addObject(MooseSharedPointer<T> object, THREAD_ID tid /*= 0*/)
{
  checkThreadID(tid);

  // Stores object in list of all objects
  _all_objects[tid].push_back(object);

  // If enabled, store object in a list of all active
  bool enabled = object->enabled();
  if (enabled)
    _active_objects[tid].push_back(object);

  // Perform casts to the Block/BoundaryRestrictable
  MooseSharedPointer<BoundaryRestrictable> bnd = MooseSharedNamespace::dynamic_pointer_cast<BoundaryRestrictable>(object);
  MooseSharedPointer<BlockRestrictable> blk = MooseSharedNamespace::dynamic_pointer_cast<BlockRestrictable>(object);

  // Boundary Restricted
  if (bnd && bnd->boundaryRestricted())
  {
    const std::set<BoundaryID> & ids = bnd->boundaryIDs();
    for (std::set<BoundaryID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
      _all_boundary_objects[tid][*it].push_back(object);
      if (enabled)
        _active_boundary_objects[tid][*it].push_back(object);
    }
  }

  // Block Restricted
  else if (blk)
  {
    const std::set<SubdomainID> & ids = blk->blockRestricted() ? blk->blockIDs() : blk->meshBlockIDs();
    for (std::set<SubdomainID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
      _all_block_objects[tid][*it].push_back(object);
      if (enabled)
        _active_block_objects[tid][*it].push_back(object);
    }
  }
}


template<typename T>
inline const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _all_objects[tid];
}


template<typename T>
inline const std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > &
MooseObjectWarehouseBase<T>::getBoundaryObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _all_boundary_objects[tid];
}


template<typename T>
const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getBoundaryObjects(BoundaryID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _all_boundary_objects[tid].find(id);
  mooseAssert(iter != _all_boundary_objects[tid].end(), "Unable to located active boundary objects for the given id: " << id << ".");
  return iter->second;
}


template<typename T>
inline const std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > &
MooseObjectWarehouseBase<T>::getBlockObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _all_block_objects[tid];
}


template<typename T>
const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getBlockObjects(SubdomainID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _all_block_objects[tid].find(id);
  mooseAssert(iter != _all_block_objects[tid].end(), "Unable to located active block objects for the given id: " << id << ".");
  return iter->second;
}


template<typename T>
inline const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getActiveObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _active_objects[tid];
}


template<typename T>
inline const std::map<BoundaryID, std::vector<MooseSharedPointer<T> > > &
MooseObjectWarehouseBase<T>::getActiveBoundaryObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _active_boundary_objects[tid];
}


template<typename T>
const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getActiveBoundaryObjects(BoundaryID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _active_boundary_objects[tid].find(id);
  mooseAssert(iter != _active_boundary_objects[tid].end(), "Unable to located active boundary objects for the given id: " << id << ".");
  return iter->second;
}


template<typename T>
inline const std::map<SubdomainID, std::vector<MooseSharedPointer<T> > > &
MooseObjectWarehouseBase<T>::getActiveBlockObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return _active_block_objects[tid];
}


template<typename T>
const std::vector<MooseSharedPointer<T> > &
MooseObjectWarehouseBase<T>::getActiveBlockObjects(SubdomainID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _active_block_objects[tid].find(id);
  mooseAssert(iter != _active_block_objects[tid].end(), "Unable to located active block objects for the given id: " << id << ".");
  return iter->second;
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  return !_active_objects[tid].empty();
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBlockObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_block_objects = false;
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
  for (it = _active_block_objects[tid].begin(); it != _active_block_objects[tid].end(); ++it)
    has_active_block_objects |= !(it->second.empty());
  return has_active_block_objects;
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBlockObjects(SubdomainID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _active_block_objects[tid].find(id);
  return iter != _active_block_objects[tid].end();
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBoundaryObjects(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_boundary_objects = false;
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
  for (it = _active_boundary_objects[tid].begin(); it != _active_boundary_objects[tid].end(); ++it)
    has_active_boundary_objects |= !(it->second.empty());
  return has_active_boundary_objects;
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator iter = _active_boundary_objects[tid].find(id);
  return iter != _active_boundary_objects[tid].end();
}


template<typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveObject(const std::string & name, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    if ((*it)->name() == name)
      return true;
  return false;
}


template<typename T>
MooseSharedPointer<T>
MooseObjectWarehouseBase<T>::getActiveObject(const std::string & name, THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    if ((*it)->name() == name)
      return *it;
  mooseError("Unable to locate active object: " << name << ".");
}


template<typename T>
std::set<SubdomainID>
MooseObjectWarehouseBase<T>::getActiveBlocks(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  std::set<SubdomainID> ids;
  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
  for (it = _active_block_objects[tid].begin(); it != _active_block_objects[tid].end(); ++it)
    ids.insert(it->first);
  return ids;
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateActive(THREAD_ID tid /*= 0*/)
{
  checkThreadID(tid);

  updateActiveHelper(_active_objects[tid], _all_objects[tid]);

  {
    typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
    for (it = _all_block_objects[tid].begin(); it != _all_block_objects[tid].end(); ++it)
      updateActiveHelper(_active_block_objects[tid][it->first], it->second);
  }

  {
    typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
    for (it = _all_boundary_objects[tid].begin(); it != _all_boundary_objects[tid].end(); ++it)
      updateActiveHelper(_active_boundary_objects[tid][it->first], it->second);
  }

}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateActiveHelper(std::vector<MooseSharedPointer<T> > & active, const std::vector<MooseSharedPointer<T> > & all)
{
  typename std::vector<MooseSharedPointer<T> >::const_iterator iter;

  // Clear the active list
  active.clear();

  // Add "enabled" objects to the active list
  for (iter = all.begin(); iter != all.end(); ++iter)
    if ( (*iter)->enabled() )
      active.push_back(*iter);
}


template<typename T>
void
MooseObjectWarehouseBase<T>::sort(THREAD_ID tid/* = 0*/)
{
  checkThreadID(tid);

  {
    typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::iterator iter;
    for (iter = _all_block_objects[tid].begin(); iter != _all_block_objects[tid].end(); ++iter)
      sortHelper(iter->second);
  }

  {
    typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::iterator iter;
    for (iter = _all_boundary_objects[tid].begin(); iter != _all_boundary_objects[tid].end(); ++iter)
      sortHelper(iter->second);
  }

  sortHelper(_all_objects[tid]);

  // The active lists now must be update to reflect the order changes
  updateActive(tid);
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateVariableDependency(std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid/* = 0*/) const
{
  if (hasActiveObjects(tid))
    updateVariableDependencyHelper(needed_moose_vars, _all_objects[tid]);
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateBlockVariableDependency(SubdomainID id, std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid/* = 0*/) const
{
  if (hasActiveBlockObjects(id, tid))
    updateVariableDependencyHelper(needed_moose_vars, getActiveBlockObjects(id, tid));
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryVariableDependency(std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid/* = 0*/) const
{
  if (hasActiveBoundaryObjects(tid))
  {
    typename std::map<BoundaryID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
    for (it = _active_boundary_objects[tid].begin(); it != _active_boundary_objects[tid].end(); ++it)
      updateVariableDependencyHelper(needed_moose_vars, it->second);
  }
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryVariableDependency(BoundaryID id, std::set<MooseVariable *> & needed_moose_vars, THREAD_ID tid/* = 0*/) const
{
  if (hasActiveBoundaryObjects(id, tid))
    updateVariableDependencyHelper(needed_moose_vars, getActiveBoundaryObjects(id, tid));
}


template<typename T>
void
MooseObjectWarehouseBase<T>::updateVariableDependencyHelper(std::set<MooseVariable *> & needed_moose_vars,
                                                                    const std::vector<MooseSharedPointer<T> > & objects)
{
  for (typename std::vector<MooseSharedPointer<T> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
  {
    const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
  }
}


template<typename T>
void
MooseObjectWarehouseBase<T>::subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables, THREAD_ID tid/*=0*/) const
{
  for (typename std::vector<MooseSharedPointer<T> >::const_iterator it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    unique_variables.insert((*it)->variable().name());

  typename std::map<SubdomainID, std::vector<MooseSharedPointer<T> > >::const_iterator it;
  for (it = _active_block_objects[tid].begin(); it != _active_block_objects[tid].end(); ++it)
    subdomains_covered.insert(it->first);
}


template<typename T>
void
MooseObjectWarehouseBase<T>::sortHelper(std::vector<MooseSharedPointer<T> > & objects)
{
  // Do nothing if the vector is empty
  if (objects.empty())
    return;

  // Make sure the object is sortable
  mooseAssert(MooseSharedNamespace::dynamic_pointer_cast<DependencyResolverInterface>(objects[0]), "Objects must inhert from DependencyResolverInterface to be sorted.");

  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort<MooseSharedPointer<T> >(objects);
  }
  catch(CyclicDependencyException<MooseSharedPointer<T> > & e)
  {
    DependencyResolverInterface::cyclicDependencyError<MooseSharedPointer<T> >(e, "Cyclic dependency detected in object ordering");
  }
}

template<typename T>
inline void
MooseObjectWarehouseBase<T>::checkThreadID(THREAD_ID tid) const
{
  mooseAssert(tid < _num_threads, "Attempting to access a thread id (" << tid << ") greater than the number allowed by the storage item (" << _num_threads << ")");
}

#endif // MOOSEOBJECTWAREHOUSEBASE_H
