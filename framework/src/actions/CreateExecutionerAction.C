//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateExecutionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "Eigenvalue.h"
#include "FEProblem.h"
#include "EigenProblem.h"

template <>
InputParameters
validParams<CreateExecutionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

CreateExecutionerAction::CreateExecutionerAction(InputParameters params) : MooseObjectAction(params)
{
}

void
CreateExecutionerAction::act()
{
  _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();

  std::shared_ptr<FEProblem> fe_problem = std::dynamic_pointer_cast<FEProblem>(_problem);
  if (fe_problem)
    _moose_object_pars.set<FEProblem *>("_fe_problem") = fe_problem.get();

  std::shared_ptr<EigenProblem> eigen_problem = std::dynamic_pointer_cast<EigenProblem>(_problem);
  if (eigen_problem)
    _moose_object_pars.set<EigenProblem *>("_eigen_problem") = eigen_problem.get();

  std::shared_ptr<Executioner> executioner =
      _factory.create<Executioner>(_type, "Executioner", _moose_object_pars);

  std::shared_ptr<Eigenvalue> eigen_executioner =
      std::dynamic_pointer_cast<Eigenvalue>(executioner);

  if ((eigen_problem == nullptr) != (eigen_executioner == nullptr))
    mooseError("Executioner is not consistent with each other; EigenExecutioner needs an "
               "EigenProblem, and Steady and Transient need a FEProblem");

  _app.executioner() = executioner;
}
