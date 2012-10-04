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

#ifndef COMPUTEELEMAUXVARSTHREAD_H
#define COMPUTEELEMAUXVARSTHREAD_H

#include "ThreadedElementLoop.h"
#include "AuxWarehouse.h"
// libMesh includes
#include "elem_range.h"

class FEProblem;
class AuxiliarySystem;


class ComputeElemAuxVarsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeElemAuxVarsThread(FEProblem & problem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x, Threads::split split);

  virtual void subdomainChanged();
  virtual void onElement(const Elem *elem);
  virtual void post();

  void join(const ComputeElemAuxVarsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;
  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTEELEMAUXVARSTHREAD_H
