//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMActionComponent.h"
#include "Control.h"

InputParameters
THMActionComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  return params;
}

THMActionComponent::THMActionComponent(const InputParameters & params) : ActionComponent(params) {}

void
THMActionComponent::actOnAdditionalTasks()
{
  if (_current_task == "THM:add_component")
    addTHMComponents();
  if (_current_task == "THM:add_closures")
    addClosures();
  if (_current_task == "THM:add_control_logic")
    addControlLogic();
}

THMProblem &
THMActionComponent::getTHMProblem()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    return *thm_problem;
  else
    mooseError("The Problem must derive from THMProblem to use THMActionComponents. Please include "
               "a [Components] block in your input file, even if empty.");
}

void
THMActionComponent::addTHMComponent(const std::string & class_name,
                                    const std::string & obj_name,
                                    InputParameters & params)
{
  auto & thm_problem = getTHMProblem();
  params.set<THMProblem *>("_thm_problem") = &thm_problem;
  thm_problem.addComponent(class_name, obj_name, params);
}

void
THMActionComponent::addClosuresObject(const std::string & class_name,
                                      const std::string & obj_name,
                                      InputParameters & params)
{
  auto & thm_problem = getTHMProblem();
  params.set<THMProblem *>("_thm_problem") = &thm_problem;
  params.set<Logger *>("_logger") = &(thm_problem.log());
  thm_problem.addClosures(class_name, obj_name, params);
}

void
THMActionComponent::addControlLogicObject(const std::string & class_name,
                                          const std::string & obj_name,
                                          InputParameters & params)
{
  auto & thm_problem = getTHMProblem();
  params.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
  params.set<THMProblem *>("_thm_problem") = &thm_problem;
  std::shared_ptr<Control> control = _factory.create<Control>(class_name, obj_name, params);
  thm_problem.getControlWarehouse().addObject(control);
}
