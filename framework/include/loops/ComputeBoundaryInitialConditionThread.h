//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedNodeLoop.h"

// MOOSE includes
#include "MooseMesh.h" // ConstBndNodeRange

// Forward declarations
class FEProblemBase;

class ComputeBoundaryInitialConditionThread
  : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeBoundaryInitialConditionThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x,
                                        Threads::split split);

  // Set IC on specific variable names
  ComputeBoundaryInitialConditionThread(FEProblemBase & fe_problem,
                                        const std::set<std::string> & target_var_names);

  void onNode(ConstBndNodeRange::const_iterator & nd);

  void join(const ComputeBoundaryInitialConditionThread & /*y*/);

protected:
  /// @brief the names of target variables for which the initial conditions are applied
  const std::set<std::string> _target_var_names;
};
