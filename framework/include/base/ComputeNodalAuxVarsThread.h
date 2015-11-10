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

#ifndef COMPUTENODALAUXVARSTHREAD_H
#define COMPUTENODALAUXVARSTHREAD_H

#include "ThreadedNodeLoop.h"
#include "AuxWarehouse.h"

// libMesh includes
#include "libmesh/node_range.h"

class AuxiliarySystem;

class ComputeNodalAuxVarsThread : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalAuxVarsThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split);

  void onNode(ConstNodeRange::const_iterator & nd);

  void join(const ComputeNodalAuxVarsThread & /*y*/);

protected:
  AuxiliarySystem & _sys;

  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTENODALAUXVARSTHREAD_H
