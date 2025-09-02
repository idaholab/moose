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
#include "ThreadedNodeLoop.h"

#include "libmesh/node_range.h"

// Forward declarations
class AuxiliarySystem;
class FEProblemBase;
template <typename T>
class MooseObjectWarehouse;

template <typename AuxKernelType>
class ComputeNodalAuxVarsThread final
  : public ThreadedNodeLoop<ConstNodeRange,
                            ConstNodeRange::const_iterator,
                            ComputeNodalAuxVarsThread<AuxKernelType>>
{
public:
  ComputeNodalAuxVarsThread(FEProblemBase & fe_problem,
                            const MooseObjectWarehouse<AuxKernelType> & storage);

  // Splitting Constructor
  ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split);

  void onNode(ConstNodeRange::const_iterator & nd);

  void join(const ComputeNodalAuxVarsThread & /*y*/);

  void subdomainChanged();

  void post();

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernelType> & _storage;

  std::set<SubdomainID> _block_ids;

  static Threads::spin_mutex writable_variable_mutex;
};
