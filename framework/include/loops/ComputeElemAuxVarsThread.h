//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEELEMAUXVARSTHREAD_H
#define COMPUTEELEMAUXVARSTHREAD_H

#include "libmesh/elem_range.h"

// MOOSE includes
#include "ThreadedElementLoop.h"
#include "MooseObjectWarehouse.h"
#include "AuxKernel.h"

// Forward declarations
class FEProblemBase;
class AuxiliarySystem;

class ComputeElemAuxVarsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeElemAuxVarsThread(FEProblemBase & problem,
                           const MooseObjectWarehouse<AuxKernel> & storage,
                           bool need_materials);
  // Splitting Constructor
  ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x, Threads::split split);

  virtual ~ComputeElemAuxVarsThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void post() override;

  void join(const ComputeElemAuxVarsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernel> & _aux_kernels;

  bool _need_materials;
};

#endif // COMPUTEELEMAUXVARSTHREAD_H
