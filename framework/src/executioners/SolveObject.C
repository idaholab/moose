//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolveObject.h"

#include "Executioner.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

SolveObject::SolveObject(Executioner & ex)
  : MooseObject(ex.parameters()),
    PerfGraphInterface(this),
    PostprocessorInterface(this),
    _executioner(ex),
    _problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _displaced_problem(_problem.getDisplacedProblem()),
    _mesh(_problem.mesh()),
    _displaced_mesh(_displaced_problem ? &_displaced_problem->mesh() : nullptr),
    _nl(_problem.getNonlinearSystemBase()),
    _aux(_problem.getAuxiliarySystem()),
    _inner_solve(nullptr)
{
}
