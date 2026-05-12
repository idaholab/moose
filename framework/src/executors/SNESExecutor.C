//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SNESExecutor.h"
#include "FEProblem.h"

#include "libmesh/petsc_solver_exception.h"

InputParameters
SNESExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params += Moose::PetscSupport::flagAndPairOptions();
  params.addParam<ExecutorName>(
      "nl_preconditioning",
      "Name of a SNESExecutor to use as the nonlinear preconditioner for this solver.");
  return params;
}

SNESExecutor::SNESExecutor(const InputParameters & params)
  : Executor(params), _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  if (isParamSetByUser("nl_preconditioning"))
  {
    auto & npc = getExecutorByName(getParam<ExecutorName>("nl_preconditioning"));
    _npc_executor = dynamic_cast<SNESExecutor *>(&npc);
    if (!_npc_executor)
      mooseError("nl_preconditioning must refer to a SNESExecutor-derived object, '",
                 getParam<ExecutorName>("nl_preconditioning"),
                 "' is not one.");
    _npc_executor->setIsNPC(true);
  }
}

SNESExecutor::~SNESExecutor()
{
  if (_snes)
    PetscCallAbort(this->comm().get(), SNESDestroy(&_snes));
  if (_vec_func)
    PetscCallAbort(this->comm().get(), VecDestroy(&_vec_func));
}

SNES
SNESExecutor::getSNES()
{
  if (!_snes_setup_done)
    setupSNES();
  return _snes;
}
