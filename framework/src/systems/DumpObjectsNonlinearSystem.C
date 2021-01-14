//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DumpObjectsNonlinearSystem.h"
#include "FEProblemBase.h"

DumpObjectsNonlinearSystem::DumpObjectsNonlinearSystem(FEProblemBase & problem,
                                                       const std::string & name)
  : NonlinearSystemBase(
        problem, problem.es().add_system<TransientNonlinearImplicitSystem>(name), name),
    _dummy(nullptr)
{
}
