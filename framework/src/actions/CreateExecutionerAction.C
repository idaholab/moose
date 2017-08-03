/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CreateExecutionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "EigenExecutioner.h"
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
  _moose_object_pars.set<FEProblem *>("_fe_problem") =
      (std::dynamic_pointer_cast<FEProblem>(_problem)).get();
  _moose_object_pars.set<EigenProblem *>("_eigen_problem") =
      (std::dynamic_pointer_cast<EigenProblem>(_problem)).get();
  std::shared_ptr<Executioner> executioner =
      _factory.create<Executioner>(_type, "Executioner", _moose_object_pars);

  if (std::dynamic_pointer_cast<EigenProblem>(_problem).get() &&
      !(std::dynamic_pointer_cast<EigenExecutioner>(executioner).get()))
    mooseError("EigenProblem requires an EigenExecutioner or its derived subclass");

  _app.executioner() = executioner;
}
