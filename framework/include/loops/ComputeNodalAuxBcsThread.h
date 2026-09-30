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
#include "TheWarehouse.h"

// Forward declarations
class AuxiliarySystem;

template <typename AuxKernelType>
class ComputeNodalAuxBcsThread
  : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeNodalAuxBcsThread(FEProblemBase & fe_problem, const TheWarehouse::Query & query);

  // Splitting Constructor
  ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split);

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalAuxBcsThread & /*y*/);

protected:
  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const override;

  AuxiliarySystem & _aux_sys;

  /// Warehouse to retrieve the auxkernels
  const TheWarehouse::Query _query;
  TheWarehouse::QueryCache<AttribThread, AttribBoundaries> _query_boundary;
};
