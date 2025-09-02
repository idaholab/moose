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

class ComputeBoundaryInitialConditionThread final
  : public ThreadedNodeLoop<ConstBndNodeRange,
                            ConstBndNodeRange::const_iterator,
                            ComputeBoundaryInitialConditionThread>
{
public:
  // Set IC on all variables
  ComputeBoundaryInitialConditionThread(FEProblemBase & fe_problem);
  // Splitting Constructor
  ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x,
                                        Threads::split split);

  void onNode(ConstBndNodeRange::const_iterator & nd);

  void join(const ComputeBoundaryInitialConditionThread & /*y*/);

private:
  /// @brief the names of target variables for which the initial conditions are applied
  const std::set<VariableName> * _target_vars = nullptr;

  /// Set IC on specific variables
  ComputeBoundaryInitialConditionThread(FEProblemBase & fe_problem,
                                        const std::set<VariableName> * target_vars);

  // Allow FEProblemBase to access the private constructor
  friend class FEProblemBase;
};
