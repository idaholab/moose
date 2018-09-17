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
  ComputeUserObjectsThread(
      FEProblemBase & problem,
      SystemBase & sys,
      const MooseObjectWarehouse<ElementUserObject> & elemental_user_objects,
      const MooseObjectWarehouse<SideUserObject> & side_user_objects,
      const MooseObjectWarehouse<InterfaceUserObject> & interface_user_objects,
      const MooseObjectWarehouse<InternalSideUserObject> & internal_side_user_objects);
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

  ///@{
  /// Storage for UserObjects (see FEProblemBase::computeUserObjects)
  const MooseObjectWarehouse<ElementUserObject> & _elemental_user_objects;
  const MooseObjectWarehouse<SideUserObject> & _side_user_objects;
  const MooseObjectWarehouse<InterfaceUserObject> & _interface_user_objects;
  const MooseObjectWarehouse<InternalSideUserObject> & _internal_side_user_objects;
  ///@}
};

#endif // COMPUTEUSEROBJECTSTHREAD_H
