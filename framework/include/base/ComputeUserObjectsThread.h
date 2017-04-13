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

#ifndef COMPUTEUSEROBJECTSTHREAD_H
#define COMPUTEUSEROBJECTSTHREAD_H

// MOOSE includes
#include "ThreadedElementLoop.h"

// libMesh includes
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
      const MooseObjectWarehouse<InternalSideUserObject> & internal_side_user_objects);
  // Splitting Constructor
  ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split);

  virtual ~ComputeUserObjectsThread();

  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void post() override;
  virtual void subdomainChanged() override;

  void join(const ComputeUserObjectsThread & /*y*/);

protected:
  const NumericVector<Number> & _soln;

  ///@{
  /// Storage for UserObjects (see FEProblemBase::computeUserObjects)
  const MooseObjectWarehouse<ElementUserObject> & _elemental_user_objects;
  const MooseObjectWarehouse<SideUserObject> & _side_user_objects;
  const MooseObjectWarehouse<InternalSideUserObject> & _internal_side_user_objects;
  ///@}
};

#endif // COMPUTEUSEROBJECTSTHREAD_H
