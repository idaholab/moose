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
#include "ThreadedElementLoop.h"
#include "ExecFlagEnum.h"
#include "AuxiliarySystem.h"

#include "libmesh/elem_range.h"

#include <map>

class InternalSideUserObject;
class ElementUserObject;
class ShapeElementUserObject;
class InterfaceUserObject;
class DomainUserObject;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Class for threaded computation of UserObjects.
 */
class ComputeUserObjectsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeUserObjectsThread(FEProblemBase & problem,
                           SystemBase & sys,
                           const TheWarehouse::Query & query);
  // Splitting Constructor
  ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split);

  virtual ~ComputeUserObjectsThread();

  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void post() override;
  virtual void subdomainChanged() override;

  void join(const ComputeUserObjectsThread & /*y*/);

protected:
  const NumericVector<Number> & _soln;

  /// Print general information about the loop, like the ordering of class of objects
  void printGeneralExecutionInformation() const override;

  /// Print information about the loop, mostly order of execution of particular objects
  void printBlockExecutionInformation() const override;

  /// Format output of vector of UOs
  template <typename T>
  void printVectorOrdering(std::vector<T *> uos, const std::string & name) const;

private:
  template <typename T>
  void querySubdomain(Interfaces iface, std::vector<T> & results)
  {
    _query_subdomain.queryInto(results, _tid, _subdomain, iface);
  }
  template <typename T>
  void queryBoundary(Interfaces iface, BoundaryID bnd, std::vector<T> & results)
  {
    _query_boundary.queryInto(results, _tid, std::make_tuple(bnd, false), iface);
  }

  const TheWarehouse::Query _query;
  TheWarehouse::QueryCache<AttribThread, AttribSubdomains, AttribInterfaces> _query_subdomain;
  TheWarehouse::QueryCache<AttribThread, AttribBoundaries, AttribInterfaces> _query_boundary;
  std::vector<InternalSideUserObject *> _internal_side_objs;
  std::vector<InterfaceUserObject *> _interface_user_objects;
  std::vector<ElementUserObject *> _element_objs;
  std::vector<ShapeElementUserObject *> _shape_element_objs;
  std::vector<DomainUserObject *> _domain_objs;
  std::vector<DomainUserObject *> _all_domain_objs;

  AuxiliarySystem & _aux_sys;
};

// determine when we need to run user objects based on whether any initial conditions or aux
// kernels depend on the user objects.  If so we need to run them either before ics, before aux
// kernels, or after aux kernels (if nothing depends on them).  Mark/store this information as
// attributes in the warehouse for later reference.
template <typename T>
void
groupUserObjects(TheWarehouse & w,
                 AuxiliarySystem & aux,
                 const ExecFlagEnum & execute_flags,
                 const std::vector<T *> & objs,
                 const std::set<std::string> & ic_deps)
{
  // These flags indicate when a user object will be executed for a given exec flag time.
  // The attributes are set by this function and their values are queried in
  // FEProblemBase::computeUserObjectsInternal(). If a UO is found to be in one of the
  // three groups: PRE_IC, PRE_AUX, or POST_AUX, then that UO is executed with that group.
  //
  // PRE_IC objects are run before initial conditions during the "INITIAL" exec flag time.
  // On any other exec flag time, they are run in POST_AUX by default or if there is
  // an dependency for some exec flag or if force_preaux is set, they are run in
  // PRE_AUX
  //
  // PRE_AUX objects are run before the dependent AuxKernels exec flag
  //
  // POST_AUX objects are run after AuxKernels on any given exec flag time, and is the
  // default group for UOs. Dependencies that would otherwise move a UO into the
  // PRE_AUX group can be overridden by specifying the parameter force_postaux
  //
  // This function attempts to sort a UO based on any ICs or AuxKernels which depend on
  // it. Alternatively, a user may select which group to execute their object with by
  // controlling the force_preic, force_preaux and force_postaux input parameters.
  //

  std::map<T *, std::set<int>> pre_aux_dependencies;
  std::map<T *, std::set<int>> post_aux_dependencies;
  // This map is used to indicate, after all dependencies have
  // been looked through, whether the UO has been flagged to
  // execute on EXEC_INITIAL, either through a dependency or
  // because force_preic was indicated. If neither of these
  // are true, the UO needs to be run in POST_AUX for EXEC_INITIAL
  std::map<T *, bool> is_pre_ic;

  for (const auto obj : objs)
    is_pre_ic[obj] = false;

  for (const ExecFlagType & flag : execute_flags.items())
  {
    std::set<std::string> depend_objects_aux = aux.getDependObjects(flag);
    for (const auto obj : objs)
    {
      if (depend_objects_aux.count(obj->name()) > 0)
      {
        pre_aux_dependencies[obj].insert(flag);
        if (flag == EXEC_INITIAL)
          is_pre_ic.at(obj) = true;
      }
      else if (flag != EXEC_INITIAL)
        // default is for UO to be post_aux. If EXEC_INITIAL, check first if UO
        // will be dependent on IC or have force_preic before deciding to put in
        // post_aux
        post_aux_dependencies[obj].insert(flag);
    }
  }

  for (const auto obj : objs)
  {
    if (ic_deps.count(obj->name()) > 0 ||
        (obj->isParamValid("force_preic") && obj->template getParam<bool>("force_preic")))
    {
      w.update(obj, AttribPreIC(w, true));
      is_pre_ic.at(obj) = true;
    }

    if ((obj->isParamValid("force_preaux") && obj->template getParam<bool>("force_preaux")))
    {
      post_aux_dependencies[obj].clear();
      for (const ExecFlagType & flag : execute_flags.items())
        pre_aux_dependencies[obj].insert(flag);
    }
    else if (obj->isParamValid("force_postaux") && obj->template getParam<bool>("force_postaux"))
    {
      pre_aux_dependencies[obj].clear();
      for (const ExecFlagType & flag : execute_flags.items())
        post_aux_dependencies[obj].insert(flag);
    }
    else
    {
      // If at this point, then check if the UO has already been set to execute
      // by either the force_preic param, an IC dependency, or a dependency
      // already found for exec flage EXEC_INITIAL. If none of these are true,
      // then is_pre_ic.at(obj) is false and the UO is added to the default
      // post_aux group for the EXEC_INITIAL flag
      if (!is_pre_ic.at(obj))
        post_aux_dependencies[obj].insert(EXEC_INITIAL);
    }
  }

  for (auto & item : pre_aux_dependencies)
    w.update(item.first, AttribPreAux(w, item.second));

  for (auto & item : post_aux_dependencies)
    w.update(item.first, AttribPostAux(w, item.second));
}
