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

#include "AddVectorPostprocessorAction.h"
#include "Factory.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddVectorPostprocessorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddVectorPostprocessorAction::AddVectorPostprocessorAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
AddVectorPostprocessorAction::act()
{
  if (!_problem)
    mooseError("The Problem has not been initialized yet!");

  _problem->addVectorPostprocessor(_type, _name, _moose_object_pars);
}
