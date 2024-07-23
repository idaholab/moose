//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddParallelAcquisitionAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "ParallelAcquisitionFunctionBase.h"

registerMooseAction("StochasticToolsApp", AddParallelAcquisitionAction, "add_parallelacquisition");

InputParameters
AddParallelAcquisitionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds acquistion function objects.");
  return params;
}

AddParallelAcquisitionAction::AddParallelAcquisitionAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddParallelAcquisitionAction::act()
{
  _problem->addObject<ParallelAcquisitionFunctionBase>(
      _type, _name, _moose_object_pars, /* threaded = */ false);
}
