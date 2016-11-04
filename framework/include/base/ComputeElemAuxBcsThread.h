
/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COMPUTEELEMAUXBCSTHREAD_H
#define COMPUTEELEMAUXBCSTHREAD_H

// MOOSE includes
#include "ParallelUniqueId.h"
#include "MooseObjectWarehouse.h"
#include "MooseMesh.h"

// Forward declarations
class FEProblem;
class AuxiliarySystem;
class AuxKernel;

class ComputeElemAuxBcsThread
{
public:
  ComputeElemAuxBcsThread(FEProblem & problem, const MooseObjectWarehouse<AuxKernel> & storage, bool need_materials);
  // Splitting Constructor
  ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x, Threads::split split);

  void operator()(const ConstBndElemRange & range);

  void join(const ComputeElemAuxBcsThread & /*y*/);

protected:
  FEProblem & _problem;
  AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernel> & _storage;

  bool _need_materials;
};

#endif //COMPUTEELEMAUXBCSTHREAD_H
