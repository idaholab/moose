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

class AuxiliarySystem;

class ComputeMarkerThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeMarkerThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  ComputeMarkerThread(ComputeMarkerThread & x, Threads::split split);

  virtual ~ComputeMarkerThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void postElement(const Elem * /*elem*/) override;
  virtual void post() override;

  void join(const ComputeMarkerThread & /*y*/);

protected:
  /// Print information about the loop
  void printGeneralExecutionInformation() const override;

  /// Print information about ordering of objects on each block
  void printBlockExecutionInformation() const override;

  FEProblemBase & _fe_problem;
  AuxiliarySystem & _aux_sys;

  /// Reference to the Marker warhouse in FEProblemBase
  const MooseObjectWarehouse<Marker> & _marker_whs;
};
