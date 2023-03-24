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
#include "ThreadedElementLoop.h"

#include "libmesh/elem_range.h"

// Forward declarations
class NonlinearSystemBase;
class ElementDamper;
template <typename T>
class MooseObjectWarehouse;

class ComputeElemDampingThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeElemDampingThread(FEProblemBase & feproblem);

  // Splitting Constructor
  ComputeElemDampingThread(ComputeElemDampingThread & x, Threads::split split);

  virtual ~ComputeElemDampingThread();

  virtual void onElement(const Elem * elem) override;

  void join(const ComputeElemDampingThread & y);

  Real damping();

protected:
  /// Print list of objects executed and in which order
  void printGeneralExecutionInformation() const override;

  Real _damping;
  NonlinearSystemBase & _nl;
  const MooseObjectWarehouse<ElementDamper> & _element_dampers;
};
