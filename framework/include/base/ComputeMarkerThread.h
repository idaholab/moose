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

#ifndef COMPUTEMARKERTHREAD_H
#define COMPUTEMARKERTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "elem_range.h"

class AuxiliarySystem;

class ComputeMarkerThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeMarkerThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<MarkerWarehouse> & marker_whs);

  // Splitting Constructor
  ComputeMarkerThread(ComputeMarkerThread & x, Threads::split split);

  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem *elem, unsigned int side);
  virtual void postElement(const Elem * /*elem*/);

  void join(const ComputeMarkerThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  AuxiliarySystem & _aux_sys;
  std::vector<MarkerWarehouse> & _marker_whs;
};

#endif //COMPUTEMARKERTHREAD_H
