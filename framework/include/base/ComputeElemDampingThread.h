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

#ifndef COMPUTEELEMDAMPINGTHREAD_H
#define COMPUTEELEMDAMPINGTHREAD_H

// MOOSE includes
#include "ThreadedElementLoop.h"
#include "MooseObjectWarehouse.h"

// libMesh includes
#include "libmesh/elem_range.h"

// Forward declarations
class NonlinearSystemBase;
class ElementDamper;

class ComputeElemDampingThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeElemDampingThread(FEProblemBase & feproblem);

  // Splitting Constructor
  ComputeElemDampingThread(ComputeElemDampingThread & x, Threads::split split);

  virtual ~ComputeElemDampingThread();

  virtual void onElement(const Elem * elem) override;

  virtual void subdomainChanged() override;

  void join(const ComputeElemDampingThread & y);

  Real damping();

  virtual void post() override;

protected:
  Real _damping;
  NonlinearSystemBase & _nl;
  const MooseObjectWarehouse<ElementDamper> & _element_dampers;
};

#endif //COMPUTEELEMDAMPINGTHREAD_H
