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

#include "libmesh/elem_range.h"

class InternalSideUserObject;
class ElementUserObject;
class ShapeElementUserObject;
class InterfaceUserObject;

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
};

// determine when we need to run user objects based on whether any initial conditions or aux
// kernels depend on the user objects.  If so we need to run them either before ics, before aux
// kernels, or after aux kernels (if nothing depends on them).  Mark/store this information as
// attributes in the warehouse for later reference.
template <typename T>
void
groupUserObjects(TheWarehouse & w,
                 const std::vector<T *> & objs,
                 const std::set<std::string> & ic_deps,
                 const std::set<std::string> & aux_deps)
{
  // These flags indicate when a user object will be executed for a given exec flag time.
  // The attributes are set by this function and their values are queried in
  // FEProblemBase::computeUserObjectsInternal(). If a UO is found to be in one of the
  // three groups: PRE_IC, PRE_AUX, or POST_AUX, then that UO is executed with that group.
  //
  // PRE_IC objects are run before initial conditions during the "INITIAL" exec flag time.
  // On any other exec flag time, they are run along with the POST_AUX group after
  // AuxKernels have ran.
  //
  // PRE_AUX objects are run before AuxKernels on any given exec flag time.
  //
  // POST_AUX objects are run after AuxKernels on any given exec flag time, and is the
  // default group for UOs.
  //
  // This function attempts to sort a UO based on any ICs or AuxKernels which depend on
  // it. Alternatively, a user may select which group to execute their object with by
  // controlling the force_preic and force_preaux input parameters.
  //
  for (const auto obj : objs)
  {
    // any object shall, by default, be post-aux unless it is assigned to another below
    w.update(obj, AttribPostAux(w, true));

    if (ic_deps.count(obj->name()) > 0 ||
        (obj->isParamValid("force_preic") && obj->template getParam<bool>("force_preic")))
    {
      w.update(obj, AttribPreIC(w, true));
      w.update(obj, AttribPostAux(w, false));
    }

    if ((obj->isParamValid("force_preaux") && obj->template getParam<bool>("force_preaux")) ||
        aux_deps.count(obj->name()) > 0 || ic_deps.count(obj->name()) > 0)
    {
      w.update(obj, AttribPreAux(w, true));
      w.update(obj, AttribPostAux(w, false));
    }
  }
}
