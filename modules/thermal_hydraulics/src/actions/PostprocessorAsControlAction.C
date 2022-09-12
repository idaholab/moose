//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorAsControlAction.h"
#include "Simulation.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", PostprocessorAsControlAction, "add_postprocessor");

InputParameters
PostprocessorAsControlAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "This action adds a control object that copies a postprocessor value into the control "
      "system so that users can work with the postprocessor name directly.");
  return params;
}

PostprocessorAsControlAction::PostprocessorAsControlAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
PostprocessorAsControlAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    const std::string class_name = "THMAddControlAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "CopyPostprocessorValueControl";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, _name + "_copy_ctrl", params));

    action->getObjectParams().set<PostprocessorName>("postprocessor") = _name;

    _awh.addActionBlock(action);
  }
}
