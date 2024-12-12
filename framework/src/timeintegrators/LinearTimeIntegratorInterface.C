//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearTimeIntegratorInterface.h"
#include "FEProblem.h"
#include "LinearSystem.h"

#include "libmesh/linear_implicit_system.h"

LinearTimeIntegratorInterface::LinearTimeIntegratorInterface(SystemBase & system)
  : _linear_system(dynamic_cast<LinearSystem *>(&system)),
    _linear_implicit_system(
        _linear_system ? dynamic_cast<LinearImplicitSystem *>(&_linear_system->system()) : nullptr)
{
}

Real
LinearTimeIntegratorInterface::timeDerivativeRHSContribution(
    const dof_id_type /*dof_id*/, const std::vector<Real> & /*factors*/) const
{
  mooseError("The time derivative right hand side contribution has not been implemented yet",
             _linear_system ? " for time integrator of system " + _linear_system->name() : "",
             "!");
}

Real
LinearTimeIntegratorInterface::timeDerivativeMatrixContribution(const Real /*factor*/) const
{
  mooseError("The time derivative matrix contribution has not been implemented yet",
             _linear_system ? " for time integrator of system " + _linear_system->name() : "",
             "!");
}
