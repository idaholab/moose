//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

// Forward declarations
template <typename T>
class MooseObjectWarehouse;

template <typename AuxKernelType>
class ComputeNodalAuxBcsThread final
  : public ThreadedNodeLoop<ConstBndNodeRange,
                            ConstBndNodeRange::const_iterator,
                            ComputeNodalAuxBcsThread<AuxKernelType>>
{
public:
  ComputeNodalAuxBcsThread(FEProblemBase & fe_problem,
                           const MooseObjectWarehouse<AuxKernelType> & storage);

  // Splitting Constructor
  ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split);

  void onNode(ConstBndNodeRange::const_iterator & node_it);

  void join(const ComputeNodalAuxBcsThread & /*y*/);

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernelType> & _storage;
};
