//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MeshDisplacedInterface.h"
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

class UserObjectBase : public MooseObject,
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
                       protected MeshDisplacedInterface,
                       protected PerfGraphInterface,
                       public DependencyResolverInterface
{
public:
  static InputParameters validParams();

  UserObjectBase(const InputParameters & params);
  virtual ~UserObjectBase() = default;

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  UserObjectBase(const UserObjectBase & object, const Moose::Kokkos::FunctorCopy & key);
#endif

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
   * Deteremine the value of a variable according to the parallel
   * maximum of the provided proxy.
   * @param[in] proxy maximum proxy will be selected
   * @param[in] value value to be obtained corresponding to the location of maximum proxy
   */
  template <typename T1, typename T2>
  void gatherProxyValueMax(T1 & proxy, T2 & value);

  /**
   * Determine the value of a variable according to which process has the parallel
   * minimum of the provided proxy.
   * @param[in] proxy minimum proxy will be selected
   * @param[in] value value to be obtained corresponding to the location of minimum proxy
   */
  template <typename T1, typename T2>
  void gatherProxyValueMin(T1 & proxy, T2 & value);

  /**
   * Recursively return a set of user objects this user object depends on
   * Note: this can be called only after all user objects are constructed.
   */
  std::set<UserObjectName> getDependObjects() const;

  const std::set<std::string> & getRequestedItems() override { return _depend_uo; }

  const std::set<std::string> & getSuppliedItems() override { return _supplied_uo; }

  /**
   * @returns the number of the system associated with this object
   */
  unsigned int systemNumber() const { return _sys.number(); }

protected:
  virtual void addPostprocessorDependencyHelper(const PostprocessorName & name) const override;
  virtual void
  addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const override;
  virtual void addUserObjectDependencyHelper(const UserObject & uo) const override;
  void addReporterDependencyHelper(const ReporterName & reporter_name) override;

  /// Thread ID of this postprocessor
  const THREAD_ID _tid;

  /// Reference to the Subproblem for this user object
  SubProblem & _subproblem;

  /// Reference to the FEProblemBase for this user object
  FEProblemBase & _fe_problem;

  /// Reference to the system object for this user object. This should correspond to a nonlinear
  /// system (either through the FEProblemBase or the DisplacedProblem)
  SystemBase & _sys;

  /// Reference to the assembly object for this user object
  Assembly & _assembly;

  /// Whether to execute this object twice on initial
  const bool _duplicate_initial_execution;

  /// Depend UserObjects that to be used both for determining user object sorting and by AuxKernel
  /// for finding the full UO dependency
  mutable std::set<std::string> _depend_uo;

private:
  /// A name of the "supplied" user objects, which is just this object
  std::set<std::string> _supplied_uo;
};

template <typename T1, typename T2>
void
UserObjectBase::gatherProxyValueMax(T1 & proxy, T2 & value)
{
  // Get all proxy, value pairs. _communicator.maxloc would be faster but leads to
  // partitioning dependent results if the maximum proxy is not unique.
  std::vector<std::pair<T1, T2>> all(n_processors());
  const auto pair = std::make_pair(proxy, value);
  _communicator.allgather(pair, all);

  // find maximum, disambiguated by the value
  const auto it = std::max_element(all.begin(), all.end());
  proxy = it->first;
  value = it->second;
}

template <typename T1, typename T2>
void
UserObjectBase::gatherProxyValueMin(T1 & proxy, T2 & value)
{
  // get all proxy, value pairs
  std::vector<std::pair<T1, T2>> all(n_processors());
  const auto pair = std::make_pair(proxy, value);
  _communicator.allgather(pair, all);

  // find minimum, disambiguated by the value
  const auto it = std::min_element(all.begin(), all.end());
  proxy = it->first;
  value = it->second;
}
