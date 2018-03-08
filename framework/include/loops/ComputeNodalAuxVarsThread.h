//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTENODALAUXVARSTHREAD_H
#define COMPUTENODALAUXVARSTHREAD_H

// MOOSE includes
#include "ThreadedNodeLoop.h"

#include "libmesh/node_range.h"

// Forward declarations
class AuxiliarySystem;
class AuxKernel;
class FEProblemBase;
template <typename T>
class MooseObjectWarehouse;

class ComputeNodalAuxVarsThread
    : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalAuxVarsThread(FEProblemBase & fe_problem,
                            const MooseObjectWarehouse<AuxKernel> & storage);
  // Splitting Constructor
  ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split);

  void onNode(ConstNodeRange::const_iterator & nd);

  void join(const ComputeNodalAuxVarsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernel> & _storage;
};

#endif // COMPUTENODALAUXVARSTHREAD_H
