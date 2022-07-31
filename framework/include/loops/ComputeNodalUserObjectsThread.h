//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedNodeLoop.h"

#include "libmesh/node_range.h"

// Forward declarations
class SubProblem;

class ComputeNodalUserObjectsThread
  : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalUserObjectsThread(FEProblemBase & fe_problem, const TheWarehouse::Query & query);
  // Splitting Constructor
  ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split split);

  virtual ~ComputeNodalUserObjectsThread();

  virtual void onNode(ConstNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalUserObjectsThread & /*y*/);

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const override;

private:
  const TheWarehouse::Query _query;
  AuxiliarySystem & _aux_sys;

  static Threads::spin_mutex writable_variable_mutex;
};
