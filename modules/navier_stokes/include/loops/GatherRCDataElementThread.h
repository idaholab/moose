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

class INSFVMomentumResidualObject;

/**
 * A class that gathers body force data from elemental kernels contributing to the Navier-Stokes
 * momentum residuals. We loop over each active, local element and call the gatherRCData method on
 * each kernel with the current element as an argument
 */
class GatherRCDataElementThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  GatherRCDataElementThread(FEProblemBase & fe_problem, const std::vector<unsigned int> & vars);

  // Splitting Constructor
  GatherRCDataElementThread(GatherRCDataElementThread & x, Threads::split split);

  void join(const GatherRCDataElementThread &) {}

protected:
  void subdomainChanged() override final;
  void onElement(const Elem * elem) override final;

private:
  /// The velocity variable numbers
  const std::vector<unsigned int> & _vars;

  /// The collection of elemental kernels that contributive to the momentum equation residuals
  std::vector<INSFVMomentumResidualObject *> _insfv_elemental_kernels;
};
