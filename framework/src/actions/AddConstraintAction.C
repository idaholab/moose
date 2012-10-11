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

#include "AddConstraintAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddConstraintAction>()
{
  return validParams<MooseObjectAction>();
}

AddConstraintAction::AddConstraintAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddConstraintAction::act()
{
  _problem->addConstraint(_type, getShortName(), _moose_object_pars);
}
