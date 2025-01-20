//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SIMPLESolve.h"
#include "FEProblem.h"
#include "SegregatedSolverUtils.h"
#include "LinearSystem.h"

using namespace libMesh;

InputParameters
SIMPLESolve::validParams()
{
  InputParameters params = LinearAssemblySegregatedSolve::validParams();
  return params;
}

SIMPLESolve::SIMPLESolve(Executioner & ex) : LinearAssemblySegregatedSolve(ex) {}

void
SIMPLESolve::checkTimeKernels(LinearSystem & system)
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (system.containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation in system",
               system.name(),
               ", SIMPLE is a steady-state solver! Use the PIMPLE executioner instead.");
}

void
SIMPLESolve::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  for (const auto system : _momentum_systems)
    checkTimeKernels(*system);

  checkTimeKernels(_pressure_system);

  if (_has_energy_system)
    checkTimeKernels(*_energy_system);

  if (_has_passive_scalar_systems)
    for (const auto system : _passive_scalar_systems)
      checkTimeKernels(*system);
}
