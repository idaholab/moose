//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseMesh.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

class FEProblemBase;

class ComputeFVInitialConditionThread
{
public:
  ComputeFVInitialConditionThread(FEProblemBase & fe_problem);
  // Splitting Constructor
  ComputeFVInitialConditionThread(ComputeFVInitialConditionThread & x, Threads::split split);
  using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
  void operator()(const ElemInfoRange & range);
  void join(const ComputeFVInitialConditionThread & /*y*/);

protected:
  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

  FEProblemBase & _fe_problem;
  THREAD_ID _tid;
};
