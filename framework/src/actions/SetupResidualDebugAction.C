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

#include "SetupResidualDebugAction.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "Factory.h"

template<>
InputParameters validParams<SetupResidualDebugAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<NonlinearVariableName> >("show_var_residual", "Variables for which residuals will be sent to the output file");
  return params;
}

SetupResidualDebugAction::SetupResidualDebugAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _show_var_residual(getParam<std::vector<NonlinearVariableName> >("show_var_residual"))
{
}

SetupResidualDebugAction::~SetupResidualDebugAction()
{
}

void
SetupResidualDebugAction::act()
{
  if (_problem == NULL)
    return;

  _problem->getNonlinearSystem().debuggingResiduals(true);

  // debug variable residuals
  for (std::vector<NonlinearVariableName>::const_iterator it = _show_var_residual.begin(); it != _show_var_residual.end(); ++it)
  {
    NonlinearVariableName var_name = *it;

    // add aux-variable
    MooseVariable & var = _problem->getVariable(0, var_name);
    const std::set<SubdomainID> & subdomains = var.activeSubdomains();

    std::stringstream aux_var_ss;
    aux_var_ss << "residual_" << var.name();
    std::string aux_var_name = aux_var_ss.str();

    if (subdomains.empty())
      _problem->addAuxVariable(aux_var_name, FEType(FIRST, LAGRANGE));
    else
      _problem->addAuxVariable(aux_var_name, FEType(FIRST, LAGRANGE), &subdomains);

    // add aux-kernel
    std::stringstream kern_ss;
    kern_ss << "residual_" << var.name() << "_kernel";
    std::string kern_name = kern_ss.str();

    InputParameters params = _factory.getValidParams("DebugResidualAux");
    params.set<AuxVariableName>("variable") = aux_var_name;
    params.set<NonlinearVariableName>("debug_variable") = var.name();
    params.set<MooseEnum>("execute_on") = "residual";
    _problem->addAuxKernel("DebugResidualAux", kern_name, params);
    params.set<MooseEnum>("execute_on") = "timestep";
    _problem->addAuxKernel("DebugResidualAux", kern_name, params);
  }
}
