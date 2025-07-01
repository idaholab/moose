//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "FixedPointSolve.h"
#include "DefaultNonlinearConvergence.h"
#include "DefaultFixedPointConvergence.h"

registerMooseAction("MooseApp", AddDefaultConvergenceAction, "add_default_nonlinear_convergence");
registerMooseAction("MooseApp",
                    AddDefaultConvergenceAction,
                    "add_default_multiapp_fixed_point_convergence");

InputParameters
AddDefaultConvergenceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds default Convergence objects to the simulation.");
  return params;
}

AddDefaultConvergenceAction::AddDefaultConvergenceAction(const InputParameters & params)
  : Action(params)
{
}

void
AddDefaultConvergenceAction::act()
{
  if (_current_task == "add_default_nonlinear_convergence")
    addDefaultNonlinearConvergence();
  else if (_current_task == "add_default_multiapp_fixed_point_convergence")
    addDefaultMultiAppFixedPointConvergence();
}

void
AddDefaultConvergenceAction::addDefaultNonlinearConvergence()
{
  if (_problem->needToAddDefaultNonlinearConvergence())
  {
    const std::string default_name = "default_nonlinear_convergence";
    // Create a default convergence for every nonlinear system
    std::vector<ConvergenceName> default_name_vec;
    for (const auto & nl_sys_name : _problem->getNonlinearSystemNames())
      default_name_vec.push_back(default_name + nl_sys_name);
    _problem->setNonlinearConvergenceNames(default_name_vec);
    _problem->addDefaultNonlinearConvergence(getMooseApp().getExecutioner()->parameters());
  }

  checkUnusedNonlinearConvergenceParameters();
}

void
AddDefaultConvergenceAction::addDefaultMultiAppFixedPointConvergence()
{
  if (_problem->needToAddDefaultMultiAppFixedPointConvergence())
  {
    const std::string conv_name = "default_multiapp_fixed_point_convergence";
    _problem->setMultiAppFixedPointConvergenceName(conv_name);
    _problem->addDefaultMultiAppFixedPointConvergence(getMooseApp().getExecutioner()->parameters());
  }

  checkUnusedMultiAppFixedPointConvergenceParameters();
}

void
AddDefaultConvergenceAction::checkUnusedNonlinearConvergenceParameters()
{
  // Only perform this check if the executioner uses Convergence objects
  auto & executioner_params = getMooseApp().getExecutioner()->parameters();
  if (!executioner_params.have_parameter<std::vector<ConvergenceName>>("nonlinear_convergence"))
    return;

  // Convergences may exist but be inactive
  bool has_convergence = false;
  for (const auto & cv_name : _problem->getNonlinearConvergenceNames())
    if (_problem->hasConvergence(cv_name))
      has_convergence = true;
  if (!has_convergence)
    return;

  // If a single convergence is a `DefaultNonlinearConvergence` they can handle the Executioner
  // parameters pertaining to the nonlinear system solve
  bool has_a_default_nl_conv = false;
  for (const auto & cv_name : _problem->getNonlinearConvergenceNames())
  {
    if (!_problem->hasConvergence(cv_name))
      continue;
    auto & conv = _problem->getConvergence(cv_name);
    auto * default_nl_conv = dynamic_cast<DefaultNonlinearConvergence *>(&conv);
    if (default_nl_conv)
      has_a_default_nl_conv = true;
  }

  // Only Convergence objects deriving from DefaultNonlinearConvergence should
  // share parameters with the executioner.
  if (!has_a_default_nl_conv)
  {
    for (const auto & cv_name : _problem->getNonlinearConvergenceNames())
    {
      if (!_problem->hasConvergence(cv_name))
        continue;

      auto nl_params = FEProblemSolve::feProblemDefaultConvergenceParams();
      std::vector<std::string> unused_params;
      for (const auto & param : nl_params.getParametersList())
        if (executioner_params.isParamSetByUser(param))
          unused_params.push_back(param);

      if (unused_params.size() > 0)
      {
        std::stringstream msg;
        msg << "The following nonlinear convergence parameters were set in the executioner, but "
               "are not used:\n";
        for (const auto & param : unused_params)
          msg << "  " << param << "\n";
        mooseError(msg.str());
      }
    }
  }
}

void
AddDefaultConvergenceAction::checkUnusedMultiAppFixedPointConvergenceParameters()
{
  // Abort check if executioner does not allow Convergence objects
  auto & executioner_params = getMooseApp().getExecutioner()->parameters();
  if (!executioner_params.have_parameter<ConvergenceName>("multiapp_fixed_point_convergence"))
    return;

  // Abort if there is no fixed point convergence. For example, Executors may not have them.
  if (!_problem->hasSetMultiAppFixedPointConvergenceName())
    return;

  const auto conv_name = _problem->getMultiAppFixedPointConvergenceName();

  // Abort check if Convergence is inactive
  if (!_problem->hasConvergence(conv_name))
    return;

  // If the convergence is a DefaultFixedPointConvergence they can handle the Executioner
  // parameters pertaining to the fixed point solve
  auto & conv = _problem->getConvergence(conv_name);
  auto * default_conv = dynamic_cast<DefaultFixedPointConvergence *>(&conv);

  // Only Convergence objects deriving from DefaultFixedPointConvergence should
  // share parameters with the executioner
  if (!default_conv)
  {
    auto fp_params = FixedPointSolve::fixedPointDefaultConvergenceParams();
    std::vector<std::string> unused_params;
    for (const auto & param : fp_params.getParametersList())
      if (executioner_params.isParamSetByUser(param))
        unused_params.push_back(param);

    if (unused_params.size() > 0)
    {
      std::stringstream msg;
      msg << "The following fixed point convergence parameters were set in the executioner, but "
             "are not used:\n";
      for (const auto & param : unused_params)
        msg << "  " << param << "\n";
      mooseError(msg.str());
    }
  }
}
