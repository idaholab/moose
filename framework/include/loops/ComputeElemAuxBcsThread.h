//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "MooseMesh.h"
#include "MooseTypes.h"

// Forward declarations
class FEProblemBase;
class AuxiliarySystem;

template <typename AuxKernelType>
class ComputeElemAuxBcsThread
{
public:
  ComputeElemAuxBcsThread(FEProblemBase & fe_problem,
                          const MooseObjectWarehouse<AuxKernelType> & storage,
                          bool need_materials);
  // Splitting Constructor
  ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x, Threads::split split);

  void operator()(const ConstBndElemRange & range);

  void join(const ComputeElemAuxBcsThread & /*y*/);

protected:
  FEProblemBase & _fe_problem;
  AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernelType> & _storage;

  bool _need_materials;

  /// Print list of object types executed and in which order
  void printGeneralExecutionInformation() const;

  /// Print list of specific objects executed and in which order
  void printBoundaryExecutionInformation(
      unsigned int boundary_id, const std::vector<std::shared_ptr<AuxKernelType>> & kernels) const;

  /// Keeps track of which boundaries the loop has reported execution on
  mutable std::set<SubdomainID> _boundaries_exec_printed;
};
