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
#include "ReporterInterface.h"
#include "MeshChangedInterface.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "Restartable.h"
#include "MeshMetaDataInterface.h"
#include "ScalarCoupleable.h"
#include "SetupInterface.h"
#include "PerfGraphInterface.h"
#include "SamplerInterface.h"

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
                   protected ReporterInterface,
                   protected DistributionInterface,
                   protected SamplerInterface,
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

      // Add dependencies of other objects, but don't allow it to call itself. This can happen
      // through the PostprocessorInterface if a Postprocessor calls getPostprocessorValueByName
      // with it's own name. This happens in the Receiver, which could use the FEProblem version of
      // the get method, but this is a fix that prevents an infinite loop occurring by accident for
      // future objects.
      if (uo.name() != name())
      {
        auto uos = uo.getDependObjects();
        for (auto & t : uos)
          all.insert(t);
      }
    }
    return all;
  }

  template <typename T>
  const T & getUserObject(const std::string & param_name) const
  {
    const auto & uo = UserObjectInterface::getUserObject<T>(param_name);
    _depend_uo.insert(_pars.get<UserObjectName>(param_name));
    return uo;
  }

  template <typename T>
  const T & getUserObjectByName(const UserObjectName & object_name) const
  {
    const auto & uo = UserObjectInterface::getUserObjectByName<T>(object_name);
    _depend_uo.insert(object_name);
    return uo;
  }

  const UserObject & getUserObjectBase(const UserObjectName & param_name) const
  {
    const auto & uo = UserObjectInterface::getUserObjectBase(param_name);
    _depend_uo.insert(uo.name());
    return uo;
  }

  const UserObject & getUserObjectBaseByName(const UserObjectName & object_name) const
  {
    const auto & uo = UserObjectInterface::getUserObjectBaseByName(object_name);
    _depend_uo.insert(object_name);
    return uo;
  }

  virtual const PostprocessorValue & getPostprocessorValue(const std::string & name,
                                                           unsigned int index = 0) const
  {
    if (!isDefaultPostprocessorValue(name, index)) // if default, no dependencies to add
      _depend_uo.insert(getPostprocessorName(name, index));
    return PostprocessorInterface::getPostprocessorValue(name, index);
  }

  virtual const PostprocessorValue &
  getPostprocessorValueByName(const PostprocessorName & name) const
  {
    _depend_uo.insert(name);
    return PostprocessorInterface::getPostprocessorValueByName(name);
  }

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & name,
                              const std::string & vector_name) const override
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
  }

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name) const override
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
  }

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValue(const std::string & name,
                              const std::string & vector_name,
                              bool needs_broadcast) const override
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getVectorPostprocessorValue(
        name, vector_name, needs_broadcast);
  }

  virtual const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                    const std::string & vector_name,
                                    bool needs_broadcast) const override
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getVectorPostprocessorValueByName(
        name, vector_name, needs_broadcast);
  }

  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const std::string & name,
                                     const std::string & vector_name) const override final
  {
    _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
    return VectorPostprocessorInterface::getScatterVectorPostprocessorValue(name, vector_name);
  }

  const ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueByName(const std::string & name,
                                           const std::string & vector_name) const override final
  {
    _depend_uo.insert(name);
    return VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(name,
                                                                                  vector_name);
  }

  /**
   * Whether or not a threaded copy of this object is needed when obtaining it in
   * another object, like via the UserObjectInterface.
   *
   * Derived classes should override this as needed.
   */
  virtual bool needThreadedCopy() const { return false; }

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
  mutable std::set<UserObjectName> _depend_uo;
};
