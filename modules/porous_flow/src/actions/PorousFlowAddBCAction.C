//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAddBCAction.h"
#include "PorousFlowDictator.h"
#include "NonlinearSystemBase.h"

registerMooseAction("PorousFlowApp", PorousFlowAddBCAction, "add_porous_flow_bc");

InputParameters
PorousFlowAddBCAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Action that allows adding BCs using proxy classes.");
  return params;
}

PorousFlowAddBCAction::PorousFlowAddBCAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
PorousFlowAddBCAction::act()
{
  if (_type == "PorousFlowSinkBC")
    setupPorousFlowEnthalpySink();
  // NOTE: no reason to catch unknown types here - MOOSE will catch that much earlier
}

void
PorousFlowAddBCAction::setupPorousFlowEnthalpySink()
{
  std::string nm = name();

  THREAD_ID tid = 0;
  const InputParameters & obj_pars = getObjectParams();

  // check that we have 2 porous flow variables
  const PorousFlowDictator & dictator = _problem->getUserObject<PorousFlowDictator>(
      obj_pars.get<UserObjectName>("PorousFlowDictator"));
  if (dictator.numVariables() != 2)
    mooseError(name(),
               ": Number of porous flow variables in simulation is '",
               dictator.numVariables(),
               "', but this BC can be used only with pressure and temperature.");

  if (obj_pars.isParamValid("porepressure_var"))
  {
    const std::vector<VariableName> & pressure_param =
        obj_pars.get<std::vector<VariableName>>("porepressure_var");
    if (pressure_param.size() != 1)
      mooseError(name(), ": 'porepressure_var' parameter can currently take only a single value");
  }
  const std::vector<BoundaryName> & boundary = obj_pars.get<std::vector<BoundaryName>>("boundary");
  const UserObjectName & dictator_name = obj_pars.get<UserObjectName>("PorousFlowDictator");
  const FunctionName & flux_fn_name = obj_pars.get<FunctionName>("flux_function");

  const MooseVariableFEBase & p_m_var = _problem->getNonlinearSystemBase().getVariable(tid, 0);
  const MooseVariableFEBase & T_m_var = _problem->getNonlinearSystemBase().getVariable(tid, 1);

  bool has_fluid_phase = obj_pars.isParamValid("fluid_phase");
  {
    const std::string class_name = "PorousFlowSink";
    InputParameters pars = _factory.getValidParams(class_name);
    _app.parser().extractParams(_name, pars); // extract global params
    pars.set<NonlinearVariableName>("variable") = {p_m_var.name()};
    pars.set<std::vector<BoundaryName>>("boundary") = boundary;
    pars.set<UserObjectName>("PorousFlowDictator") = dictator_name;
    pars.set<bool>("use_mobility") = false;
    pars.set<bool>("use_relperm") = false;
    if (has_fluid_phase)
      pars.set<unsigned int>("fluid_phase") = obj_pars.get<unsigned int>("fluid_phase");
    else
      pars.set<std::vector<VariableName>>("porepressure_var") =
          obj_pars.get<std::vector<VariableName>>("porepressure_var");
    pars.set<FunctionName>("flux_function") = flux_fn_name;
    _problem->addBoundaryCondition(class_name, nm + "_p_bc", pars);
  }
  {
    const std::string class_name = "PorousFlowEnthalpySink";
    InputParameters pars = _factory.getValidParams(class_name);
    _app.parser().extractParams(_name, pars); // extract global params
    pars.set<NonlinearVariableName>("variable") = {T_m_var.name()};
    pars.set<std::vector<BoundaryName>>("boundary") = boundary;
    pars.set<Real>("T_in") = obj_pars.get<Real>("T_in");
    pars.set<UserObjectName>("fp") = obj_pars.get<UserObjectName>("fp");
    pars.set<UserObjectName>("PorousFlowDictator") = dictator_name;
    if (has_fluid_phase)
      pars.set<unsigned int>("fluid_phase") = obj_pars.get<unsigned int>("fluid_phase");
    else
      pars.set<std::vector<VariableName>>("porepressure_var") =
          obj_pars.get<std::vector<VariableName>>("porepressure_var");
    pars.set<FunctionName>("flux_function") = flux_fn_name;
    _problem->addBoundaryCondition(class_name, nm + "_T_bc", pars);
  }
}
