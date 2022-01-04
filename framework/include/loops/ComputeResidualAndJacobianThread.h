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
#include "MooseObjectTagWarehouse.h"

#include "libmesh/elem_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class IntegratedBCBase;
class DGKernelBase;
class InterfaceKernelBase;
class TimeKernel;
class KernelBase;
class Kernel;

class ComputeResidualAndJacobianThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeResidualAndJacobianThread(FEProblemBase & fe_problem,
                                   const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & /*matrix_tags*/);

  // Splitting Constructor
  ComputeResidualAndJacobianThread(ComputeResidualAndJacobianThread & x, Threads::split split);

  virtual ~ComputeResidualAndJacobianThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void postElement(const Elem * /*elem*/) override;

  void join(const ComputeResidualAndJacobianThread & /*y*/);

protected:
  NonlinearSystemBase & _nl;
  const std::set<TagID> & _tags;
  unsigned int _num_cached;
};
