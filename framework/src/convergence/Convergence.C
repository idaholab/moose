//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Convergence.h"

InputParameters
Convergence::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();
  params += PerfGraphInterface::validParams();

  params.registerBase("Convergence");
  // params.registerSystemAttributeName("ConvergenceName");
  params.addClassDescription("Check convergence of the set up problem.");

  return params;
}

Convergence::Convergence(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    PerfGraphInterface(this),
    _perf_nonlinear(
        registerTimedSection("checkNonlinearConvergence", 5, "Checking Nonlinear Convergence"))
{
}
