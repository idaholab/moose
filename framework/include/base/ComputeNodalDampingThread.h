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

#ifndef COMPUTENODALDAMPINGTHREAD_H
#define COMPUTENODALDAMPINGTHREAD_H

// MOOSE includes
#include "ThreadedNodeLoop.h"

// Forward declarations
template <typename T>
class MooseObjectWarehouse;
class NodalDamper;
class NonlinearSystemBase;

class ComputeNodalDampingThread
    : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalDampingThread(FEProblemBase & feproblem);

  // Splitting Constructor
  ComputeNodalDampingThread(ComputeNodalDampingThread & x, Threads::split split);

  virtual ~ComputeNodalDampingThread();

  virtual void onNode(ConstNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalDampingThread & y);

  Real damping();

protected:
  Real _damping;
  NonlinearSystemBase & _nl;
  const MooseObjectWarehouse<NodalDamper> & _nodal_dampers;
};

#endif // COMPUTENODALDAMPINGTHREAD_H
