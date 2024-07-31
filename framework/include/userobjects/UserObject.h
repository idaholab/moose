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

class FEProblemBase;
class SubProblem;
class Assembly;
class SystemBase;

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
                   protected PerfGraphInterface,
                   public DependencyResolverInterface
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
   * Optional interface function for providing the points at which a UserObject attains
   * spatial values. If a UserObject overrides this function, then other objects that
   * take both the UserObject and points can instead directly use the points specified
   * on the UserObject.
   */
  virtual const std::vector<Point> spatialPoints() const
  {
    mooseError("Spatial UserObject interface is not satisfied; spatialPoints() must be overridden");
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

  /**
   * Gather the parallel max of the variable passed in. It takes care of values across all threads
   * and CPUs (we DO hybrid parallelism!)
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherMax(T & value)
  {
    _communicator.max(value);
  }

  /**
   * Gather the parallel min of the variable passed in. It takes care of values across all threads
   * and CPUs (we DO hybrid parallelism!)
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherMin(T & value)
  {
    _communicator.min(value);
  }

  /**
   * Gather the parallel value of a variable according to which process has the parallel
   * maximum of the provided value.
   * @param[in] value process with maximum value will be selected
   * @param[in] proxy value to be obtained on process with maximum value
   */
  template <typename T1, typename T2>
  void gatherProxyValueMax(T1 & value, T2 & proxy);

  /**
   * Gather the parallel value of a variable according to which process has the parallel
   * minimum of the provided value.
   * @param[in] value process with minimum value will be selected
   * @param[in] proxy value to be obtained on process with minimum value
   */
  template <typename T1, typename T2>
  void gatherProxyValueMin(T1 & value, T2 & proxy);

  void setPrimaryThreadCopy(UserObject * primary);

  UserObject * primaryThreadCopy() { return _primary_thread_copy; }

  /**
   * Recursively return a set of user objects this user object depends on
   * Note: this can be called only after all user objects are constructed.
   */
  std::set<UserObjectName> getDependObjects() const;

  /**
   * Whether or not a threaded copy of this object is needed when obtaining it in
   * another object, like via the UserObjectInterface.
   *
   * Derived classes should override this as needed.
   */
  virtual bool needThreadedCopy() const { return false; }

  const std::set<std::string> & getRequestedItems() override { return _depend_uo; }

  const std::set<std::string> & getSuppliedItems() override { return _supplied_uo; }

  /**
   * @returns the number of the system associated with this object
   */
  unsigned int systemNumber() const;

protected:
  virtual void addPostprocessorDependencyHelper(const PostprocessorName & name) const override;
  virtual void
  addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const override;
  virtual void addUserObjectDependencyHelper(const UserObject & uo) const override;
  void addReporterDependencyHelper(const ReporterName & reporter_name) override;

  /// Reference to the Subproblem for this user object
  SubProblem & _subproblem;

  /// Reference to the FEProblemBase for this user object
  FEProblemBase & _fe_problem;

  /// Reference to the system object for this user object. This should correspond to a nonlinear
  /// system (either through the FEProblemBase or the DisplacedProblem)
  SystemBase & _sys;

  /// Thread ID of this postprocessor
  const THREAD_ID _tid;
  Assembly & _assembly;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  const bool _duplicate_initial_execution;

  /// Depend UserObjects that to be used both for determining user object sorting and by AuxKernel
  /// for finding the full UO dependency
  mutable std::set<std::string> _depend_uo;

private:
  UserObject * _primary_thread_copy = nullptr;

  /// A name of the "supplied" user objects, which is just this object
  std::set<std::string> _supplied_uo;
};

template <typename T1, typename T2>
void
UserObject::gatherProxyValueMax(T1 & value, T2 & proxy)
{
  // get all value, proxy pairs, _communicator.maxloc would be faster but leads to
  // partitioning dependent results if the maximum value is not unique.
  std::vector<std::pair<T1, T2>> all(n_processors());
  auto pair = std::make_pair(value, proxy);
  _communicator.allgather(pair, all);

  // find maximum, disambiguated by the proxy value
  const auto it = std::max_element(all.begin(), all.end());
  value = it->first;
  proxy = it->second;
}

template <typename T1, typename T2>
void
UserObject::gatherProxyValueMin(T1 & value, T2 & proxy)
{
  // get all value, proxy pairs
  std::vector<std::pair<T1, T2>> all(n_processors());
  auto pair = std::make_pair(value, proxy);
  _communicator.allgather(pair, all);

  // find minimum, disambiguated by the proxy value
  const auto it = std::min_element(all.begin(), all.end());
  value = it->first;
  proxy = it->second;
}
