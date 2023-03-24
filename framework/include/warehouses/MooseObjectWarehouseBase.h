//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "DependencyResolverInterface.h"
#include "BoundaryRestrictable.h"
#include "BlockRestrictable.h"
#include "TransientInterface.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "SubProblem.h"

// Forward declarations
class MooseVariableFieldBase;

/**
 * A base storage container for MooseObjects.
 */
template <typename T>
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
   * @param tid The thread ID (default is 0)
   * @param recurse Whether or not to build recusive warehouses (typically for Kernels)
   */
  virtual void addObject(std::shared_ptr<T> object, THREAD_ID tid = 0, bool recurse = true);

  /**
   * Return how many kernels we store in the current warehouse
   */
  unsigned int size(THREAD_ID tid = 0) const;

  ///@{
  /**
   * Retrieve complete vector to the all/block/boundary restricted objects for a given thread.
   * @param tid The thread id to retrieve objects from
   */
  const std::vector<std::shared_ptr<T>> & getObjects(THREAD_ID tid = 0) const;
  const std::map<SubdomainID, std::vector<std::shared_ptr<T>>> &
  getBlockObjects(THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<T>> & getBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  const std::map<BoundaryID, std::vector<std::shared_ptr<T>>> &
  getBoundaryObjects(THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<T>> & getBoundaryObjects(BoundaryID id,
                                                             THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Retrieve complete vector to the active all/block/boundary restricted objects for a given
   * thread.
   * @param tid The thread id to retrieve objects from
   */
  const std::vector<std::shared_ptr<T>> & getActiveObjects(THREAD_ID tid = 0) const;
  const std::map<SubdomainID, std::vector<std::shared_ptr<T>>> &
  getActiveBlockObjects(THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<T>> & getActiveBlockObjects(SubdomainID id,
                                                                THREAD_ID tid = 0) const;
  const std::map<BoundaryID, std::vector<std::shared_ptr<T>>> &
  getActiveBoundaryObjects(THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<T>> & getActiveBoundaryObjects(BoundaryID id,
                                                                   THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Convenience functions for determining if objects exist.
   */
  bool hasObjects(THREAD_ID tid = 0) const;
  bool hasActiveObjects(THREAD_ID tid = 0) const;
  bool hasActiveBlockObjects(THREAD_ID tid = 0) const;
  bool hasActiveBlockObjects(SubdomainID id, THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(THREAD_ID tid = 0) const;
  bool hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
  bool hasBoundaryObjects(BoundaryID id, THREAD_ID tid = 0) const;
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
  std::shared_ptr<T> getObject(const std::string & name, THREAD_ID tid = 0) const;
  std::shared_ptr<T> getActiveObject(const std::string & name, THREAD_ID tid = 0) const;
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
  void updateVariableDependency(std::set<MooseVariableFieldBase *> & needed_moose_vars,
                                THREAD_ID tid = 0) const;
  void updateBlockVariableDependency(SubdomainID id,
                                     std::set<MooseVariableFieldBase *> & needed_moose_vars,
                                     THREAD_ID tid = 0) const;
  void updateBoundaryVariableDependency(std::set<MooseVariableFieldBase *> & needed_moose_vars,
                                        THREAD_ID tid = 0) const;
  void updateBoundaryVariableDependency(BoundaryID id,
                                        std::set<MooseVariableFieldBase *> & needed_moose_vars,
                                        THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Update FE variable coupleable vector tag vector
   */
  void updateBlockFEVariableCoupledVectorTagDependency(SubdomainID id,
                                                       std::set<TagID> & needed_fe_var_vector_tags,
                                                       THREAD_ID tid = 0) const;
  void updateBoundaryFEVariableCoupledVectorTagDependency(
      BoundaryID id, std::set<TagID> & needed_fe_var_vector_tags, THREAD_ID tid = 0) const;
  ///@}

  ///@{
  /**
   * Update material property dependency vector.
   */
  void updateMatPropDependency(std::set<unsigned int> & needed_mat_props, THREAD_ID tid = 0) const;
  void updateBlockMatPropDependency(SubdomainID id,
                                    std::set<unsigned int> & needed_mat_props,
                                    THREAD_ID tid = 0) const;
  void updateBoundaryMatPropDependency(std::set<unsigned int> & needed_mat_props,
                                       THREAD_ID tid = 0) const;
  void updateBoundaryMatPropDependency(BoundaryID id,
                                       std::set<unsigned int> & needed_mat_props,
                                       THREAD_ID tid = 0) const;
  ///@}

  /**
   * Populates a set of covered subdomains and the associated variable names.
   */
  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered,
                         std::set<std::string> & unique_variables,
                         THREAD_ID tid = 0) const;

  /**
   * Return the number of threads.
   */
  THREAD_ID numThreads() const { return _num_threads; }

  /**
   * Output the active content of the warehouse to a string, meant to be output to the console
   * @param tid the thread id
   * @param prefix a string to prepend to the string
   */
  std::string activeObjectsToFormattedString(THREAD_ID tid = 0,
                                             const std::string & prefix = "[DBG]") const;

protected:
  /// Convenience member storing the number of threads used for storage (1 or libMesh::n_threads)
  const THREAD_ID _num_threads;

  /// Storage container for the ALL pointers (THREAD_ID on outer vector)
  std::vector<std::vector<std::shared_ptr<T>>> _all_objects;

  /// All active objects (THREAD_ID on outer vector)
  std::vector<std::vector<std::shared_ptr<T>>> _active_objects;

  // All block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<std::shared_ptr<T>>>> _all_block_objects;

  /// Active block restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<SubdomainID, std::vector<std::shared_ptr<T>>>> _active_block_objects;

  // All boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<std::shared_ptr<T>>>> _all_boundary_objects;

  /// Active boundary restricted objects (THREAD_ID on outer vector)
  std::vector<std::map<BoundaryID, std::vector<std::shared_ptr<T>>>> _active_boundary_objects;

  /**
   * Helper method for updating active vectors
   */
  static void updateActiveHelper(std::vector<std::shared_ptr<T>> & active,
                                 const std::vector<std::shared_ptr<T>> & all);

  /**
   * Helper method for sorting vectors of objects.
   */
  static void sortHelper(std::vector<std::shared_ptr<T>> & objects);

  /**
   * Helper method for updating variable dependency vector
   */
  static void updateVariableDependencyHelper(std::set<MooseVariableFieldBase *> & needed_moose_vars,
                                             const std::vector<std::shared_ptr<T>> & objects);

  /**
   * Helper method for updating FE variable coupleable vector tag vector
   */
  static void
  updateFEVariableCoupledVectorTagDependencyHelper(std::set<TagID> & needed_fe_var_vector_tags,
                                                   const std::vector<std::shared_ptr<T>> & objects);

  /**
   * Helper method for updating material property dependency vector
   */
  static void updateMatPropDependencyHelper(std::set<unsigned int> & needed_mat_props,
                                            const std::vector<std::shared_ptr<T>> & objects);

  /**
   * Calls assert on thread id.
   */
  void checkThreadID(THREAD_ID tid) const;

  friend class MaterialWarehouse;
};

template <typename T>
MooseObjectWarehouseBase<T>::MooseObjectWarehouseBase(bool threaded /*=true*/)
  : _num_threads(threaded ? libMesh::n_threads() : 1),
    _all_objects(_num_threads),
    _active_objects(_num_threads),
    _all_block_objects(_num_threads),
    _active_block_objects(_num_threads),
    _all_boundary_objects(_num_threads),
    _active_boundary_objects(_num_threads)
{
}

template <typename T>
MooseObjectWarehouseBase<T>::~MooseObjectWarehouseBase()
{
}

template <typename T>
unsigned int
MooseObjectWarehouseBase<T>::size(THREAD_ID tid /* =0 */) const
{
  checkThreadID(tid);
  return _all_objects[tid].size();
}

template <typename T>
void
MooseObjectWarehouseBase<T>::addObject(std::shared_ptr<T> object,
                                       THREAD_ID tid /*= 0*/,
                                       bool /* recurse = true */)
{
  checkThreadID(tid);

  // Stores object in list of all objects
  _all_objects[tid].push_back(object);

  // If enabled, store object in a list of all active
  bool enabled = object->enabled();
  if (enabled)
    _active_objects[tid].push_back(object);

  // Perform casts to the Block/BoundaryRestrictable
  std::shared_ptr<BoundaryRestrictable> bnd =
      std::dynamic_pointer_cast<BoundaryRestrictable>(object);
  std::shared_ptr<BlockRestrictable> blk = std::dynamic_pointer_cast<BlockRestrictable>(object);

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
    const std::set<SubdomainID> & ids =
        blk->blockRestricted() ? blk->blockIDs() : blk->meshBlockIDs();
    for (std::set<SubdomainID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
      _all_block_objects[tid][*it].push_back(object);
      if (enabled)
        _active_block_objects[tid][*it].push_back(object);
    }

    // Check variables
    std::shared_ptr<Coupleable> c_ptr = std::dynamic_pointer_cast<Coupleable>(object);
    if (c_ptr)
      for (MooseVariableFieldBase * var : c_ptr->getCoupledMooseVars())
        blk->checkVariable(*var);

    const InputParameters & parameters = object->parameters();

    SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

    THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

    if (parameters.isParamValid("variable"))
    {
      // Try the scalar version first
      std::string variable_name = parameters.getMooseType("variable");
      if (variable_name == "")
        // When using vector variables, we are only going to use the first one in the list at the
        // interface level...
        variable_name = parameters.getVecMooseType("variable")[0];

      blk->checkVariable(problem.getVariable(
          tid, variable_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY));
    }
  }
}

template <typename T>
inline const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _all_objects[tid];
}

template <typename T>
inline const std::map<BoundaryID, std::vector<std::shared_ptr<T>>> &
MooseObjectWarehouseBase<T>::getBoundaryObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _all_boundary_objects[tid];
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasBoundaryObjects(BoundaryID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _all_boundary_objects[tid].find(id) != _all_boundary_objects[tid].end();
}

template <typename T>
const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getBoundaryObjects(BoundaryID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _all_boundary_objects[tid].find(id);
  mooseAssert(iter != _all_boundary_objects[tid].end(),
              "Unable to located active boundary objects for the given id: " << id << ".");
  return iter->second;
}

template <typename T>
inline const std::map<SubdomainID, std::vector<std::shared_ptr<T>>> &
MooseObjectWarehouseBase<T>::getBlockObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _all_block_objects[tid];
}

template <typename T>
const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getBlockObjects(SubdomainID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _all_block_objects[tid].find(id);
  mooseAssert(iter != _all_block_objects[tid].end(),
              "Unable to located active block objects for the given id: " << id << ".");
  return iter->second;
}

template <typename T>
inline const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getActiveObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _active_objects[tid];
}

template <typename T>
inline const std::map<BoundaryID, std::vector<std::shared_ptr<T>>> &
MooseObjectWarehouseBase<T>::getActiveBoundaryObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _active_boundary_objects[tid];
}

template <typename T>
const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getActiveBoundaryObjects(BoundaryID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _active_boundary_objects[tid].find(id);
  mooseAssert(iter != _active_boundary_objects[tid].end(),
              "Unable to located active boundary objects for the given id: " << id << ".");
  return iter->second;
}

template <typename T>
inline const std::map<SubdomainID, std::vector<std::shared_ptr<T>>> &
MooseObjectWarehouseBase<T>::getActiveBlockObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return _active_block_objects[tid];
}

template <typename T>
const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouseBase<T>::getActiveBlockObjects(SubdomainID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _active_block_objects[tid].find(id);
  mooseAssert(iter != _active_block_objects[tid].end(),
              "Unable to located active block objects for the given id: " << id << ".");
  return iter->second;
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return !_all_objects[tid].empty();
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  return !_active_objects[tid].empty();
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBlockObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_block_objects = false;
  for (const auto & object_pair : _active_block_objects[tid])
    has_active_block_objects |= !(object_pair.second.empty());
  return has_active_block_objects;
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBlockObjects(SubdomainID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _active_block_objects[tid].find(id);
  return iter != _active_block_objects[tid].end();
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBoundaryObjects(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  bool has_active_boundary_objects = false;
  for (const auto & object_pair : _active_boundary_objects[tid])
    has_active_boundary_objects |= !(object_pair.second.empty());
  return has_active_boundary_objects;
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _active_boundary_objects[tid].find(id);
  return iter != _active_boundary_objects[tid].end();
}

template <typename T>
bool
MooseObjectWarehouseBase<T>::hasActiveObject(const std::string & name, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    if (object->name() == name)
      return true;
  return false;
}

template <typename T>
std::shared_ptr<T>
MooseObjectWarehouseBase<T>::getObject(const std::string & name, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _all_objects[tid])
    if (object->name() == name)
      return object;
  mooseError("Unable to locate object: ", name, ".");
}

template <typename T>
std::shared_ptr<T>
MooseObjectWarehouseBase<T>::getActiveObject(const std::string & name, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    if (object->name() == name)
      return object;
  mooseError("Unable to locate active object: ", name, ".");
}

template <typename T>
std::set<SubdomainID>
MooseObjectWarehouseBase<T>::getActiveBlocks(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  std::set<SubdomainID> ids;
  for (const auto & object_pair : _active_block_objects[tid])
    ids.insert(object_pair.first);
  return ids;
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateActive(THREAD_ID tid /*= 0*/)
{
  checkThreadID(tid);

  updateActiveHelper(_active_objects[tid], _all_objects[tid]);

  for (const auto & object_pair : _all_block_objects[tid])
    updateActiveHelper(_active_block_objects[tid][object_pair.first], object_pair.second);

  for (const auto & object_pair : _all_boundary_objects[tid])
    updateActiveHelper(_active_boundary_objects[tid][object_pair.first], object_pair.second);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateActiveHelper(std::vector<std::shared_ptr<T>> & active,
                                                const std::vector<std::shared_ptr<T>> & all)
{
  // Clear the active list
  active.clear();

  std::copy_if(all.begin(),
               all.end(),
               std::back_inserter(active),
               [](const std::shared_ptr<T> & object) { return object->enabled(); });
}

template <typename T>
void
MooseObjectWarehouseBase<T>::sort(THREAD_ID tid /* = 0*/)
{
  checkThreadID(tid);

  for (auto & object_pair : _all_block_objects[tid])
    sortHelper(object_pair.second);

  for (auto & object_pair : _all_boundary_objects[tid])
    sortHelper(object_pair.second);

  sortHelper(_all_objects[tid]);

  // The active lists now must be update to reflect the order changes
  updateActive(tid);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateVariableDependency(
    std::set<MooseVariableFieldBase *> & needed_moose_vars, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveObjects(tid))
    updateVariableDependencyHelper(needed_moose_vars, _all_objects[tid]);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBlockVariableDependency(
    SubdomainID id,
    std::set<MooseVariableFieldBase *> & needed_moose_vars,
    THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBlockObjects(id, tid))
    updateVariableDependencyHelper(needed_moose_vars, getActiveBlockObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryVariableDependency(
    std::set<MooseVariableFieldBase *> & needed_moose_vars, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBoundaryObjects(tid))
  {
    typename std::map<BoundaryID, std::vector<std::shared_ptr<T>>>::const_iterator it;
    for (const auto & object_pair : _active_boundary_objects[tid])
      updateVariableDependencyHelper(needed_moose_vars, object_pair.second);
  }
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryVariableDependency(
    BoundaryID id,
    std::set<MooseVariableFieldBase *> & needed_moose_vars,
    THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBoundaryObjects(id, tid))
    updateVariableDependencyHelper(needed_moose_vars, getActiveBoundaryObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateVariableDependencyHelper(
    std::set<MooseVariableFieldBase *> & needed_moose_vars,
    const std::vector<std::shared_ptr<T>> & objects)
{
  for (const auto & object : objects)
  {
    auto c = dynamic_cast<const MooseVariableDependencyInterface *>(object.get());
    if (c)
    {
      const auto & mv_deps = c->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBlockFEVariableCoupledVectorTagDependency(
    SubdomainID id, std::set<TagID> & needed_fe_var_vector_tags, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBlockObjects(id, tid))
    updateFEVariableCoupledVectorTagDependencyHelper(needed_fe_var_vector_tags,
                                                     getActiveBlockObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryFEVariableCoupledVectorTagDependency(
    BoundaryID id, std::set<TagID> & needed_fe_var_vector_tags, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBoundaryObjects(id, tid))
    updateFEVariableCoupledVectorTagDependencyHelper(needed_fe_var_vector_tags,
                                                     getActiveBoundaryObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateFEVariableCoupledVectorTagDependencyHelper(
    std::set<TagID> & needed_fe_var_vector_tags, const std::vector<std::shared_ptr<T>> & objects)
{
  for (const auto & object : objects)
  {
    auto c = dynamic_cast<const Coupleable *>(object.get());
    if (c)
    {
      const auto & tag_deps = c->getFEVariableCoupleableVectorTags();
      needed_fe_var_vector_tags.insert(tag_deps.begin(), tag_deps.end());
    }
  }
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateMatPropDependency(std::set<unsigned int> & needed_mat_props,
                                                     THREAD_ID tid /* = 0*/) const
{
  if (hasActiveObjects(tid))
    updateMatPropDependencyHelper(needed_mat_props, _all_objects[tid]);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBlockMatPropDependency(SubdomainID id,
                                                          std::set<unsigned int> & needed_mat_props,
                                                          THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBlockObjects(id, tid))
    updateMatPropDependencyHelper(needed_mat_props, getActiveBlockObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryMatPropDependency(
    std::set<unsigned int> & needed_mat_props, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBoundaryObjects(tid))
    for (auto & active_bnd_object : _active_boundary_objects[tid])
      updateMatPropDependencyHelper(needed_mat_props, active_bnd_object.second);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateBoundaryMatPropDependency(
    BoundaryID id, std::set<unsigned int> & needed_mat_props, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBoundaryObjects(id, tid))
    updateMatPropDependencyHelper(needed_mat_props, getActiveBoundaryObjects(id, tid));
}

template <typename T>
void
MooseObjectWarehouseBase<T>::updateMatPropDependencyHelper(
    std::set<unsigned int> & needed_mat_props, const std::vector<std::shared_ptr<T>> & objects)
{
  for (auto & object : objects)
  {
    auto c = dynamic_cast<const MaterialPropertyInterface *>(object.get());
    if (c)
    {
      auto & mp_deps = c->getMatPropDependencies();
      needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
    }
  }
}

template <typename T>
void
MooseObjectWarehouseBase<T>::subdomainsCovered(std::set<SubdomainID> & subdomains_covered,
                                               std::set<std::string> & unique_variables,
                                               THREAD_ID tid /*=0*/) const
{
  for (const auto & object : _active_objects[tid])
    unique_variables.insert(object->variable().name());

  for (const auto & object_pair : _active_block_objects[tid])
    subdomains_covered.insert(object_pair.first);
}

template <typename T>
std::string
MooseObjectWarehouseBase<T>::activeObjectsToFormattedString(
    const THREAD_ID tid /*=0*/, const std::string & prefix /*="[DBG]"*/) const
{
  std::vector<std::string> output;
  for (const auto & object : _active_objects[tid])
    output.push_back(object->name());
  return ConsoleUtils::formatString(MooseUtils::join(output, " "), prefix);
}

template <typename T>
void
MooseObjectWarehouseBase<T>::sortHelper(std::vector<std::shared_ptr<T>> & objects)
{
  // Do nothing if the vector is empty
  if (objects.empty())
    return;

  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort<std::shared_ptr<T>>(objects);
  }
  catch (CyclicDependencyException<std::shared_ptr<T>> & e)
  {
    DependencyResolverInterface::cyclicDependencyError<std::shared_ptr<T>>(
        e, "Cyclic dependency detected in object ordering");
  }
}

template <typename T>
inline void
MooseObjectWarehouseBase<T>::checkThreadID(THREAD_ID libmesh_dbg_var(tid)) const
{
  mooseAssert(tid < _num_threads,
              "Attempting to access a thread id ("
                  << tid << ") greater than the number allowed by the storage item ("
                  << _num_threads << ")");
}
