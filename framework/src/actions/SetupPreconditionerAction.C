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

#include "SetupPreconditionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MoosePreconditioner.h"
#include "FEProblem.h"
#include "CreateExecutionerAction.h"

unsigned int SetupPreconditionerAction::_count = 0;

template<>
InputParameters validParams<SetupPreconditionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  CreateExecutionerAction::populateCommonExecutionerParams(params);

  return params;
}

SetupPreconditionerAction::SetupPreconditionerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
SetupPreconditionerAction::act()
{
  if (_problem != NULL)
  {
    // build the preconditioner
    _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem;
    MoosePreconditioner * pc = dynamic_cast<MoosePreconditioner *>(_factory.create(_type, getShortName(), _moose_object_pars));
    if (pc == NULL)
      mooseError("Failed to build the preconditioner.");

    _problem->getNonlinearSystem().setPreconditioner(pc);

    /**
     * Go ahead and set common precondition options here.  The child classes will still be called
     * through the action warehouse
     */
    CreateExecutionerAction::storeCommonExecutionerParams(*_problem, _pars);
  }
}
