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

// MOOSE includes
#include "AddControlAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "Control.h"

template <>
InputParameters
validParams<AddControlAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddControlAction::AddControlAction(InputParameters parameters) : MooseObjectAction(parameters) {}

void
AddControlAction::act()
{
  _moose_object_pars.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
  std::shared_ptr<Control> control = _factory.create<Control>(_type, _name, _moose_object_pars);
  _problem->getControlWarehouse().addObject(control);
}
