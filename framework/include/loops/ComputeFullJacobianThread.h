//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeJacobianThread.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class Kernel;

class ComputeFullJacobianThread : public ComputeJacobianThread
{
public:
  ComputeFullJacobianThread(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split);

  virtual ~ComputeFullJacobianThread();

  void join(const ComputeJacobianThread & /*y*/) {}

protected:
  virtual void computeOnElement() override;
  virtual void computeOnBoundary(BoundaryID bnd_id, const Elem * lower_d_elem) override;
  virtual void computeOnInternalFace(const Elem * neighbor) override;
  virtual void computeOnInterface(BoundaryID bnd_id) override;
};
