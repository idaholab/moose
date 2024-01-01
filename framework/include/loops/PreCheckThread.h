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
class HybridizedKernel;

class PreCheckThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  PreCheckThread(FEProblemBase & fe_problem,
                 MooseObjectWarehouse<HybridizedKernel> & hybridized_kernels);

  // Splitting Constructor
  PreCheckThread(PreCheckThread & x, Threads::split split);

  virtual ~PreCheckThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void post() override;

  void join(const PreCheckThread &) {}

protected:
  /// the hybridized kernels we will call the pre-check method for
  MooseObjectWarehouse<HybridizedKernel> & _hybridized_kernels;
};
