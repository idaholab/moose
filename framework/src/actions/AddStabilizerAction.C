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

#include "AddStabilizerAction.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddStabilizerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}

AddStabilizerAction::AddStabilizerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddStabilizerAction::act() 
{
  _problem->addStabilizer(_type, getShortName(), _moose_object_pars);
}
