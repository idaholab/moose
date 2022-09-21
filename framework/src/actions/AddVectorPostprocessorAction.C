//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddVectorPostprocessorAction.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddVectorPostprocessorAction, "add_vector_postprocessor");

InputParameters
AddVectorPostprocessorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a VectorPostprocessor object to the simulation.");
  return params;
}

AddVectorPostprocessorAction::AddVectorPostprocessorAction(const InputParameters & params)
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
