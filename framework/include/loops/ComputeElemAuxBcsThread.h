//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEELEMAUXBCSTHREAD_H
#define COMPUTEELEMAUXBCSTHREAD_H

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
  ComputeElemAuxBcsThread(FEProblemBase & problem,
                          const MooseObjectWarehouse<AuxKernelType> & storage,
                          const std::vector<std::map<std::string, MooseVariableFEBase *>> & vars,
                          bool need_materials);
  // Splitting Constructor
  ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x, Threads::split split);

  void operator()(const ConstBndElemRange & range);

  void join(const ComputeElemAuxBcsThread & /*y*/);

protected:
  FEProblemBase & _problem;
  AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernelType> & _storage;

  const std::vector<std::map<std::string, MooseVariableFEBase *>> & _aux_vars;

  bool _need_materials;
};

#endif // COMPUTEELEMAUXBCSTHREAD_H
