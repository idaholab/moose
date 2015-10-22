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

template<>
InputParameters validParams<CreateExecutionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}


CreateExecutionerAction::CreateExecutionerAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
CreateExecutionerAction::act()
{
  Moose::setup_perf_log.push("Create Executioner","Setup");
  _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem.get();
  MooseSharedPointer<Executioner> executioner = MooseSharedNamespace::static_pointer_cast<Executioner>(_factory.create(_type, "Executioner", _moose_object_pars));
  Moose::setup_perf_log.pop("Create Executioner","Setup");

  _app.executioner() = executioner;
}
