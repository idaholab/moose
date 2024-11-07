//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDefaultConvergenceAction.h"
#include "FEProblem.h"
#include "Executioner.h"
#include "FEProblemSolve.h"
#include "DefaultNonlinearConvergence.h"

registerMooseAction("MooseApp", AddDefaultConvergenceAction, "add_default_convergence");

InputParameters
AddDefaultConvergenceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add a default Convergence object to the simulation.");
  return params;
}

AddDefaultConvergenceAction::AddDefaultConvergenceAction(const InputParameters & params)
  : Action(params)
{
}

void
AddDefaultConvergenceAction::act()
{
  if (_problem->needToAddDefaultNonlinearConvergence())
  {
    const std::string default_name = "default_nonlinear_convergence";
    _problem->setNonlinearConvergenceName(default_name);
    _problem->addDefaultNonlinearConvergence(getMooseApp().getExecutioner()->parameters());
  }

  checkUnusedNonlinearConvergenceParameters();
}

void
AddDefaultConvergenceAction::checkUnusedNonlinearConvergenceParameters()
{
  // Only perform this check if the executioner uses Convergence objects
  auto & executioner_params = getMooseApp().getExecutioner()->parameters();
  if (!executioner_params.have_parameter<ConvergenceName>("nonlinear_convergence"))
    return;

  // Convergences may exist but be inactive
  if (!_problem->hasConvergence(_problem->getNonlinearConvergenceName()))
    return;

  // Only Convergence objects deriving from DefaultNonlinearConvergence should
  // share parameters with the executioner.
  auto & conv = _problem->getConvergence(_problem->getNonlinearConvergenceName());
  auto * default_nl_conv = dynamic_cast<DefaultNonlinearConvergence *>(&conv);
  if (!default_nl_conv)
  {
    auto nl_params = FEProblemSolve::feProblemDefaultConvergenceParams();
    std::vector<std::string> unused_params;
    for (const auto & param : nl_params.getParametersList())
      if (executioner_params.isParamSetByUser(param))
        unused_params.push_back(param);

    if (unused_params.size() > 0)
    {
      std::stringstream msg;
      msg << "The following nonlinear convergence parameters were set in the executioner, but are "
             "not used:\n";
      for (const auto & param : unused_params)
        msg << "  " << param << "\n";
      mooseError(msg.str());
    }
  }
}
