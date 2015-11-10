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

#ifndef WAREHOUSEBASE_H
#define WAREHOUSEBASE_H

// MOOSE includes
#include "MooseError.h"
#include "ExecuteOnStorage.h"

/**
 * MooseObject storage container.
 *
 * This storage warehouse is capable of storing MooseObject based objects,
 * handling libMesh::n_threads() copies.
 */
template<typename BaseType>
class WarehouseBase
{
public:

  /**
   * Constructor.
   * @param threaded When true threaded storage is enabled.
   */
  WarehouseBase(bool threaded = true);

  /**
   * Desctructor.
   */
  virtual ~WarehouseBase();

  /**
   * Add an object that is a type of the base warehouse object.
   * @param object A shared pointer to the object being added to the warehouse.
   * @param tid The thread on which to operate.
   */
  virtual void add(MooseSharedPointer<BaseType> object, THREAD_ID tid = 0);

  /**
   * Add an object of a differing type to the warehouse.
   * @tparam AddType The type of object being added.
   * @param object A shared pointer to the object being added to the warehouse.
   * @param tid The thread on which to operate.
   *
   * This method is not implemented in the WarehouseBase, but it is meant to be specialized
   * on AddType and use calls to the WarehouseBase::add method that takes a reference to a
   * storage container.
   *
   * @see UserObjectWarehouse
   */
  template<typename AddType>
  void add(MooseSharedPointer<AddType> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Return a vector of all/active objects for a given thread and execution type.
   */
  const std::vector<MooseSharedPointer<BaseType> > & get(THREAD_ID tid = 0) const;
  const std::vector<MooseSharedPointer<BaseType> > & getActive(THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Retrieve a vector of all/active object pointers based on execution flag.
   */
  const std::vector<MooseSharedPointer<BaseType> > & get(const ExecFlagType & exec_flag, THREAD_ID tid) const;
  const std::vector<MooseSharedPointer<BaseType> > & getActive(const ExecFlagType & exec_flag, THREAD_ID tid) const;
  ///@}

  ///@{
  /**
   * Loop through each object and call the setup method on each thread.
   */
  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void subdomainSetup();
  virtual void jacobianSetup();
  virtual void residualSetup();
  ///@}

  /**
   * Convenience method for calling the setup method based on ExecFlagType on each thread.
   * @param exec_flag The execution flag (e.g., EXEC_INITIAL calls initialSetup())
   */
  virtual void setup(const ExecFlagType & exec_flag);

  /**
   * Call the execute method on each object.
   * @param exec_flag The execution flag
   * @param tid The thread on which to operate.
   */
  virtual void execute(const ExecFlagType & exec_flag, THREAD_ID tid = 0);

  /**
   * Updates the active object list(s)
   */
  virtual void updateActive();

  /**
   * Return the number of objects stored.
   */
  unsigned int size();

protected:

  /**
   * Check that the supplied thread id is acceptable.
   * @param tid The thread on which to operate.
   */
  void checkThreadID(const THREAD_ID & id) const;

  /**
   * Add an object to the provided ExecuteOnStorage container.
   *
   * This method is desinged to be used by child classes that contain multiple ExecuteOnStorage
   * objects of differing type.
   */
  template<typename AddType>
  void add(ExecuteOnStorage<AddType> & storage, MooseSharedPointer<AddType> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Retrieve objects to the provided ExecuteOnStorage container.
   *
   * This method is designed to be used by child classes that contain multiple ExecuteOnStorage
   * objects of differing type.
   */
  template<typename GetType>
  const std::vector<MooseSharedPointer<GetType> > & get(const ExecuteOnStorage<GetType> & storage, const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;
  template<typename GetType>
  const std::vector<MooseSharedPointer<GetType> > & getActive(const ExecuteOnStorage<GetType> & storage, const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;
  ///@}

  /// True if the Warehouse is storing threaded copies.
  bool _threaded;

  /// Storage for all objects in the warehouse
  std::vector<std::vector<MooseSharedPointer<BaseType> > > _all_objects;

  /// Storage for all active objects in the warehouse
  std::vector<std::vector<MooseSharedPointer<BaseType> > > _active_objects;

  /// Storage for all objects base on Execution flags.
  ExecuteOnStorage<BaseType> _exec_objects;

};


template<typename BaseType>
void
WarehouseBase<BaseType>::add(MooseSharedPointer<BaseType> object, THREAD_ID tid)
{
  checkThreadID(tid);

  _all_objects[tid].push_back(object);

  if (object->enabled())
    _active_objects[tid].push_back(object);

  add<BaseType>(_exec_objects, object, tid);
}


template<typename BaseType>
template<typename AddType>
void
WarehouseBase<BaseType>::add(MooseSharedPointer<AddType> /*object*/, THREAD_ID /*tid*/)
{
  mooseError("Adding an object of the supplied type is not supported by the warehouse.");
}


template<typename BaseType>
template<typename AddType>
void
WarehouseBase<BaseType>::add(ExecuteOnStorage<AddType> & storage, MooseSharedPointer<AddType> object, THREAD_ID tid)
{
  checkThreadID(tid);

  storage.add(object, tid);
}


template<typename BaseType>
const std::vector<MooseSharedPointer<BaseType> > &
WarehouseBase<BaseType>::get(THREAD_ID tid) const
{
  return _all_objects[tid];
}


template<typename BaseType>
const std::vector<MooseSharedPointer<BaseType> > &
WarehouseBase<BaseType>::getActive(THREAD_ID tid) const
{
  return _active_objects[tid];
}


template<typename BaseType>
const std::vector<MooseSharedPointer<BaseType> > &
WarehouseBase<BaseType>::get(const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  return get<BaseType>(_exec_objects, exec_flag, tid);
}


template<typename BaseType>
const std::vector<MooseSharedPointer<BaseType> > &
WarehouseBase<BaseType>::getActive(const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  return getActive<BaseType>(_exec_objects, exec_flag, tid);
}


template<typename BaseType>
template<typename GetType>
const std::vector<MooseSharedPointer<GetType> > &
WarehouseBase<BaseType>::get(const ExecuteOnStorage<GetType> & storage, const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  checkThreadID(tid);
  return storage.get(exec_flag, tid);
}


template<typename BaseType>
template<typename GetType>
const std::vector<MooseSharedPointer<GetType> > &
WarehouseBase<BaseType>::getActive(const ExecuteOnStorage<GetType> & storage, const ExecFlagType & exec_flag, THREAD_ID tid) const
{
  checkThreadID(tid);
  return storage.getActive(exec_flag, tid);
}


template<typename BaseType>
void
WarehouseBase<BaseType>::updateActive()
{
  for (THREAD_ID tid = 0; tid < _active_objects.size(); ++tid)
  {
    _active_objects[tid].clear();

    for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = _all_objects[tid].begin(); it != _all_objects[tid].end(); ++it)
      if ( (*it)->enabled() )
        _active_objects[tid].push_back(*it);
  }

  _exec_objects.updateActive();
}


template<typename BaseType>
void
WarehouseBase<BaseType>::initialSetup()
{
  for (THREAD_ID tid = 0; tid < _active_objects.size(); ++tid)
  {
    const std::vector<MooseSharedPointer<BaseType> > & objects = _active_objects[tid];
    for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->initialSetup();
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::timestepSetup()
{
  for (THREAD_ID tid = 0; tid < _active_objects.size(); ++tid)
  {
    const std::vector<MooseSharedPointer<BaseType> > & objects = _active_objects[tid];
    for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->timestepSetup();
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::subdomainSetup()
{
  for (THREAD_ID tid = 0; tid < _active_objects.size(); ++tid)
  {
    const std::vector<MooseSharedPointer<BaseType> > & objects = _active_objects[tid];
    for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->subdomainSetup();
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::jacobianSetup()
{
  for (THREAD_ID tid = 0; tid < _exec_objects.size(); ++tid)
  {
      const std::vector<MooseSharedPointer<BaseType> > & objects = _exec_objects.getActive(EXEC_NONLINEAR, tid);
      for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->jacobianSetup();
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::residualSetup()
{
  for (THREAD_ID tid = 0; tid < _exec_objects.size(); ++tid)
  {
    const std::vector<MooseSharedPointer<BaseType> > & objects = _exec_objects.getActive(EXEC_LINEAR, tid);
    for (typename std::vector<MooseSharedPointer<BaseType> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      (*it)->residualSetup();
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::setup(const ExecFlagType & exec_flag)
{
  switch (exec_flag)
  {
  case EXEC_INITIAL:
    initialSetup();
    break;
  case EXEC_LINEAR:
    residualSetup();
    break;
  case EXEC_NONLINEAR:
    jacobianSetup();
    break;
  case EXEC_TIMESTEP_BEGIN:
    timestepSetup();
    break;
  case EXEC_SUBDOMAIN:
    subdomainSetup();
    break;
  default:
    break;
  }
}


template<typename BaseType>
void
WarehouseBase<BaseType>::execute(const ExecFlagType & /*exec_flag*/, THREAD_ID /*tid*/)
{
  mooseError("The execute method is not defined for this warehouse.");
}


template<typename BaseType>
WarehouseBase<BaseType>::WarehouseBase(bool threaded) :
    _threaded(threaded),
    _all_objects(_threaded ? libMesh::n_threads() : 1),
    _active_objects(_threaded ? libMesh::n_threads() : 1),
    _exec_objects(_threaded ? libMesh::n_threads() : 1)
{
}


template<typename BaseType>
WarehouseBase<BaseType>::~WarehouseBase()
{
}


template<typename BaseType>
void
WarehouseBase<BaseType>::checkThreadID(const THREAD_ID & tid) const
{
  if (_threaded)
    mooseAssert(tid == 0, "Attempting to add an object on thread id " << tid << " on non-threaded warehouse.");
}


template<typename BaseType>
unsigned int
WarehouseBase<BaseType>::size()
{
  return _all_objects[0].size();
}

#endif // WAREHOUSEBASE_H
