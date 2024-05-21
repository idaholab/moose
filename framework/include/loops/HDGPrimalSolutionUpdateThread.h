//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoop.h"

#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

// Forward declarations
class FEProblemBase;
template <typename>
class MooseObjectWarehouse;
class HDGKernel;

/**
 * This loop is run right after the linear solve for the Lagrange multiplier solution update, and
 * updates the primal solution using the Lagrange multiplier update
 */
class HDGPrimalSolutionUpdateThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  HDGPrimalSolutionUpdateThread(FEProblemBase & fe_problem,
                                MooseObjectWarehouse<HDGKernel> & hybridized_kernels);

  // Splitting Constructor
  HDGPrimalSolutionUpdateThread(HDGPrimalSolutionUpdateThread & x, Threads::split split);

  virtual ~HDGPrimalSolutionUpdateThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void post() override;

  void join(const HDGPrimalSolutionUpdateThread &) {}

protected:
  /// the hybridized kernels we will call the pre-check method for
  MooseObjectWarehouse<HDGKernel> & _hybridized_kernels;
};
