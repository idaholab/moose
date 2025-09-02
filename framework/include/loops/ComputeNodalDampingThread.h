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
template <typename T>
class MooseObjectWarehouse;
class NodalDamper;
class NonlinearSystemBase;

class ComputeNodalDampingThread final : public ThreadedNodeLoop<ConstNodeRange,
                                                                ConstNodeRange::const_iterator,
                                                                ComputeNodalDampingThread>
{
public:
  ComputeNodalDampingThread(FEProblemBase & feproblem, NonlinearSystemBase & nl);

  // Splitting Constructor
  ComputeNodalDampingThread(ComputeNodalDampingThread & x, Threads::split split);

  virtual ~ComputeNodalDampingThread();

  void onNode(ConstNodeRange::const_iterator & node_it);

  void join(const ComputeNodalDampingThread & y);

  Real damping();

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

protected:
  Real _damping;
  NonlinearSystemBase & _nl;
  const MooseObjectWarehouse<NodalDamper> & _nodal_dampers;
};
