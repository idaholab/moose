//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddOptimizationReporterAction.h"

#include "FEProblemBase.h"

registerMooseAction("OptimizationApp", AddOptimizationReporterAction, "add_reporter");

InputParameters
AddOptimizationReporterAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds OptimizationReporter objects for optimization routines.");
  return params;
}

AddOptimizationReporterAction::AddOptimizationReporterAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddOptimizationReporterAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}
