//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateApplicationBlockAction.h"
#include "MooseApp.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", CreateApplicationBlockAction, "create_application_block");

InputParameters
CreateApplicationBlockAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::string>(
      "type", "", "The name of the application that should run this input file.");
  // Declared here so the [Application] block accepts it; the value is read directly from the
  // parser by MooseApp at construction (well before this action's task runs), so this action
  // does not apply it. See MooseApp::n_threads().
  params.addParam<unsigned int>(
      "num_threads",
      "Caps the number of threads this application uses (must be <= the process-wide --n-threads); "
      "defaults to --n-threads.");

  params.addClassDescription("Adds application and application related parameters.");

  return params;
}

CreateApplicationBlockAction::CreateApplicationBlockAction(const InputParameters & parameters)
  : Action(parameters), _type(getParam<std::string>("type"))
{
}

void
CreateApplicationBlockAction::act()
{
}
