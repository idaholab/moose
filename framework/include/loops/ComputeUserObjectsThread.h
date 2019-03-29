//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEUSEROBJECTSTHREAD_H
#define COMPUTEUSEROBJECTSTHREAD_H

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
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
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
    _query.clone()
        .condition<AttribThread>(_tid)
        .condition<AttribSubdomains>(_subdomain)
        .condition<AttribInterfaces>(iface)
        .queryInto(results);
  }
  template <typename T>
  void queryBoundary(Interfaces iface, BoundaryID bnd, std::vector<T> & results)
  {
    _query.clone()
        .condition<AttribThread>(_tid)
        .condition<AttribBoundaries>(bnd)
        .condition<AttribInterfaces>(iface)
        .queryInto(results);
  }

  const TheWarehouse::Query _query;
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
  // Notes about how this information is used later during the simulation:
  // We only need to run pre-ic objects for their "initial" exec flag time (not the others).
  //
  // For pre/post aux objects:
  //
  //     If an object was not run as a pre-ic object or it is a pre-ic object and
  //     shouldDuplicateInitialExecution returns true:
  //         * run the object at all its exec flag times.
  //     Else
  //         * run the object at all its exec flag times *except* "initial"
  //
  for (const auto obj : objs)
  {
    if (ic_deps.count(obj->name()) > 0)
      w.update(obj, AttribPreIC(w, true));

    if ((obj->isParamValid("force_preaux") && obj->template getParam<bool>("force_preaux")) ||
        aux_deps.count(obj->name()) > 0 || ic_deps.count(obj->name()) > 0)
      w.update(obj, AttribPreAux(w, true));
    else
      w.update(obj, AttribPreAux(w, false));
  }
}

#endif // COMPUTEUSEROBJECTSTHREAD_H
