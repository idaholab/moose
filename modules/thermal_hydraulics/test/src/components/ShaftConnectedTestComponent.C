//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedTestComponent.h"
#include "FlowModelSinglePhase.h"
#include "Shaft.h"

registerMooseObject("ThermalHydraulicsTestApp", ShaftConnectedTestComponent);

InputParameters
ShaftConnectedTestComponent::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();
  params += ShaftConnectable::validParams();
  params.addClassDescription(
      "Component that shows how to connect a junction-like component to a shaft.");
  return params;
}

ShaftConnectedTestComponent::ShaftConnectedTestComponent(const InputParameters & parameters)
  : VolumeJunction1Phase(parameters),
    ShaftConnectable(this),
    _jct_var_name(Component::genName(name(), "var"))
{
}

void
ShaftConnectedTestComponent::addVariables()
{
  getTHMProblem().addSimVariable(true, _jct_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_jct_var_name, 2.3);
}

void
ShaftConnectedTestComponent::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const Component & c = getComponentByName<Component>(_shaft_name);
  const Shaft & scc = dynamic_cast<const Shaft &>(c);
  const VariableName omega_var_name = scc.getOmegaVariableName();

  {
    std::string class_name = "ADShaftConnectedTestComponentUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<UserObjectName>>("numerical_flux_names") = _numerical_flux_names;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("omega") = {omega_var_name};
    params.set<std::vector<VariableName>>("jct_var") = {_jct_var_name};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, getShaftConnectedUserObjectName(), params);
  }

  {
    const std::string class_name = "ADScalarTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _jct_var_name;
    getTHMProblem().addScalarKernel(class_name, genName(name(), _jct_var_name, "td"), params);
  }
  {
    const std::string class_name = "ADVolumeJunctionAdvectionScalarKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _jct_var_name;
    params.set<UserObjectName>("volume_junction_uo") = getShaftConnectedUserObjectName();
    params.set<unsigned int>("equation_index") = 0;
    getTHMProblem().addScalarKernel(class_name, genName(name(), _jct_var_name, "vja_sk"), params);
  }
}
