//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeFullJacobianThread.h"
#include "MooseTypes.h" // for TagID

#include "libmesh/threads.h"
#include "libmesh/elem_range.h"

class FEProblemBase;

class ComputeJacobianForScalingThread : public ComputeFullJacobianThread
{
public:
  ComputeJacobianForScalingThread(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeJacobianForScalingThread(ComputeJacobianForScalingThread & x, Threads::split split);

  void operator()(const libMesh::ConstElemRange & range, bool bypass_threading = false) final;

protected:
  virtual void computeOnElement() override;
};
