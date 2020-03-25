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
#include "DistributionInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "MeshChangedInterface.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "Restartable.h"
#include "MeshMetaDataInterface.h"
#include "ScalarCoupleable.h"
#include "SetupInterface.h"
#include "PerfGraphInterface.h"

#include "libmesh/parallel.h"

// Forward declarations
class UserObject;
class FEProblemBase;
class SubProblem;
class Assembly;

template <>
InputParameters validParams<UserObject>();

/**
 * Base class for user-specific data
 */
class UserObject : public MooseObject,
                   public SetupInterface,
                   protected FunctionInterface,
                   public UserObjectInterface,
                   protected PostprocessorInterface,
                   protected VectorPostprocessorInterface,
                   protected DistributionInterface,
                   protected Restartable,
                   protected MeshMetaDataInterface,
                   protected MeshChangedInterface,
                   protected ScalarCoupleable,
                   protected PerfGraphInterface
{
public:
  static InputParameters validParams();

  UserObject(const InputParameters & params);
  virtual ~UserObject() = default;

  /**
   * Execute method.
   */
  virtual void execute() = 0;

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() = 0;

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where
   * you want to do MPI communication!
   */
  virtual void finalize() = 0;

  /**
   * Returns a reference to the subproblem that
   * this postprocessor is tied to
   */
  SubProblem & getSubProblem() const { return _subproblem; }

  /**
   * Returns whether or not this user object should be executed twice during the initial condition
   * when depended upon by an IC.
   */
  bool shouldDuplicateInitialExecution() const { return _duplicate_initial_execution; }

  /**
   * Optional interface function for "evaluating" a UserObject at a spatial position.
   * If a UserObject overrides this function that UserObject can then be used in a
   * Transfer to transfer information from one domain to another.
   */
  virtual Real spatialValue(const Point & /*p*/) const
  {
    mooseError(name(), " does not satisfy the Spatial UserObject interface!");
  }

  /**
   * Must override.
   *
   * @param uo The UserObject to be joined into _this_ object.  Take the data from the uo object and
   * "add" it into the data for this object.
   */
  virtual void threadJoin(const UserObject & uo) = 0;

  /**
   * Gather the parallel sum of the variable passed in. It takes care of values across all threads
   * and CPUs (we DO hybrid parallelism!)
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherSum(T & value)
  {
    _communicator.sum(value);
  }

  template <typename T>
  void gatherMax(T & value)
  {
    _communicator.max(value);
  }

  template <typename T>
  void gatherMin(T & value)
  {
    _communicator.min(value);
  }

  template <typename T1, typename T2>
  void gatherProxyValueMax(T1 & value, T2 & proxy)
  {
    unsigned int rank;
    _communicator.maxloc(value, rank);
    _communicator.broadcast(proxy, rank);
  }

  void setPrimaryThreadCopy(UserObject * primary)
  {
    if (!_primary_thread_copy && primary != this)
      _primary_thread_copy = primary;
  }

  UserObject * primaryThreadCopy() { return _primary_thread_copy; }

  /**
   * Recursively return a set of user objects this user object depends on
   * Note: this can be called only after all user objects are constructed.
   */
  std::set<UserObjectName> getDependObjects() const
  {
    std::set<UserObjectName> all;
    for (auto & v : _depend_uo)
    {
      all.insert(v);
      auto & uo = UserObjectInterface::getUserObjectBaseByName(v);
      auto uos = uo.getDependObjects();
      for (auto & t : uos)
        all.insert(t);
    }
    return all;
  }

  template <typename T>
  const T & getUserObject(const std::string & name)
  {
    _depend_uo.insert(_pars.get<UserObjectName>(name));
    return UserObjectInterface::getUserObject<T>(name);
  }

  template <typename T>
  const T & getUserObjectByName(const UserObjectName & name)
  {
    _depend_uo.insert(name);
    return UserObjectInterface::getUserObjectByName<T>(name);
  }

  const UserObject & getUserObjectBase(const UserObjectName & name)
  {
    return getUserObjectBaseByName(_pars.get<UserObjectName>(name));
  }

  const UserObject & getUserObjectBaseByName(const UserObjectName & name)
  {
    _depend_uo.insert(name);
    return UserObjectInterface::getUserObjectBaseByName(name);
  }

  const PostprocessorValue & getPostprocessorValue(const std::string & name, unsigned int index = 0)
  {
    if (hasPostprocessor(name, index))
    {
      UserObjectName nm;
      if (_pars.isSinglePostprocessor(name))
        nm = _pars.get<PostprocessorName>(name);
      else
        nm = _pars.get<std::vector<PostprocessorName>>(name)[index];

      _depend_uo.insert(nm);
    }
    return PostprocessorInterface::getPostprocessorValue(name, index);
  }

  const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name)
  {
    _depend_uo.insert(name);
    return PostprocessorInterface::getPostprocessorValueByName(name);
  }

  const VectorPostprocessorValue & getVectorPostprocessorValue(const std::string & name,
                                                               const std::string & vector_name)
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
  }

  const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name)
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
  }

  const VectorPostprocessorValue & getVectorPostprocessorValue(const std::string & name,
                                                               const std::string & vector_name,
                                                               bool needs_broadcast)
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getVectorPostprocessorValue(
        name, vector_name, needs_broadcast);
  }

  const VectorPostprocessorValue & getVectorPostprocessorValueByName(
      const VectorPostprocessorName & name, const std::string & vector_name, bool needs_broadcast)
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getVectorPostprocessorValueByName(
        name, vector_name, needs_broadcast);
  }

  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const std::string & name, const std::string & vector_name)
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getScatterVectorPostprocessorValue(name, vector_name);
  }

  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueByName(const std::string & name,
                                           const std::string & vector_name)
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(name,
                                                                                  vector_name);
  }

protected:
  /// Reference to the Subproblem for this user object
  SubProblem & _subproblem;

  /// Reference to the FEProblemBase for this user object
  FEProblemBase & _fe_problem;

  /// Thread ID of this postprocessor
  THREAD_ID _tid;
  Assembly & _assembly;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  const bool _duplicate_initial_execution;

private:
  UserObject * _primary_thread_copy = nullptr;

  /// Depend UserObjects that to be used by AuxKernel for finding the full UO dependency
  std::set<UserObjectName> _depend_uo;
};
