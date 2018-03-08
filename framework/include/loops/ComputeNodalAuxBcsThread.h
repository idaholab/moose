//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTENODALAUXBCSTHREAD_H
#define COMPUTENODALAUXBCSTHREAD_H

// MOOSE includes
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

// Forward declarations
template <typename T>
class MooseObjectWarehouse;
class AuxKernel;

class ComputeNodalAuxBcsThread
    : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeNodalAuxBcsThread(FEProblemBase & fe_problem,
                           const MooseObjectWarehouse<AuxKernel> & storage);

  // Splitting Constructor
  ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split);

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalAuxBcsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernel> & _storage;
};

#endif // COMPUTENODALAUXBCSTHREAD_H
