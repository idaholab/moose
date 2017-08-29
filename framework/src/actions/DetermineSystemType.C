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

#include "DetermineSystemType.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<DetermineSystemType>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.mooseObjectSyntaxVisibility(false);
  return params;
}

DetermineSystemType::DetermineSystemType(InputParameters parameters) : MooseObjectAction(parameters)
{
}

void
DetermineSystemType::act()
{
  /**
   * Determine whether the Executioner is derived from EigenExecutionerBase and
   * set a flag on MooseApp that can be used during problem construction.
   */
  if (_moose_object_pars.isParamValid("_eigen") && _moose_object_pars.get<bool>("_eigen"))
    _app.useNonlinear() = false;

  auto _action_params = _action_factory.getValidParams("CreateProblemAction");

  if (_moose_object_pars.isParamValid("_use_eigen_value") &&
      _moose_object_pars.get<bool>("_use_eigen_value"))
  {
    _app.useEigenvalue() = true;
    _action_params.set<std::string>("type") = "EigenProblem";
  }
  else
  {
    _action_params.set<std::string>("type") = "FEProblem";
  }

  // if we have Problem block in input file, "CreateProblemAction" will be handled by parser
  if (_awh.hasActions("create_problem"))
    return;

  // Create "CreateProblemAction"
  auto action = _action_factory.create(
      "CreateProblemAction", _action_params.get<std::string>("type"), _action_params);

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}
