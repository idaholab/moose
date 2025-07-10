//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

class FEProblemBase;

class ComputeInitialConditionThread
{
public:
  ComputeInitialConditionThread(FEProblemBase & fe_problem);
  // Splitting Constructor
  ComputeInitialConditionThread(ComputeInitialConditionThread & x, Threads::split split);
  // Set IC on specific variable names
  ComputeInitialConditionThread(FEProblemBase & fe_problem,
                                const std::set<VariableName> & target_var_names,
                                const TargetVarUsageForIC target_var_usage);

  void operator()(const libMesh::ConstElemRange & range);
  void join(const ComputeInitialConditionThread & /*y*/);

protected:
  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

  FEProblemBase & _fe_problem;
  THREAD_ID _tid;

  /// @brief the names of target variables for which the initial conditions are applied
  const std::set<VariableName> _target_var_names;

  // Default behavior is to skip variable names in the list (_target_var_names).
  // It is because _target_var_names is empty by default. And, we want to apply initial conditions
  // to all variables by default.
  TargetVarUsageForIC _target_var_usage = TargetVarUsageForIC::SKIP_LIST;
};
