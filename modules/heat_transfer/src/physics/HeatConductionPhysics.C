//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionPhysics.h"
#include "ADHeatConduction.h"

registerMooseObject("HeatConductionApp", HeatConductionPhysics);

InputParameters
HeatConductionPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Add the heat conduction physics");

  params.transferParam<MaterialPropertyName>(ADHeatConduction::validParams(),
                                             "thermal_conductivity");
  params.addParam<VariableName>("temperature_name", "T", "Variable name for the temperature");
  params.addParam<VariableName>("heat_source_var", "Variable providing the heat source");

  return params;
}

HeatConductionPhysics::HeatConductionPhysics(const InputParameters & parameters)
  : PhysicsBase(parameters), _temperature_name(getParam<VariableName>("temperature_name"))
{
}

void
HeatConductionPhysics::addFEKernels()
{
  {
    const std::string kernel_type = "ADHeatConduction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.applyParameter(
        parameters(), "thermal_conductivity", /*private=*/true, /*override_default=*/true);
    getProblem().addKernel(kernel_type, name() + "_" + _temperature_name + "_conduction", params);
  }
  if (isParamValid("heat_source_var"))
  {
    const std::string kernel_type = "ADCoupledForce";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.set<std::vector<VariableName>>("v") = {getParam<VariableName>("heat_source_var")};
    getProblem().addKernel(kernel_type, name() + "_" + _temperature_name + "_source", params);
  }
  if (isTransient())
  {
    const std::string kernel_type = "TimeDerivative";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    getProblem().addKernel(kernel_type, name() + "_" + _temperature_name + "_time", params);
  }
}

void
HeatConductionPhysics::addNonlinearVariables()
{
  const std::string variable_type = "MooseVariable";
  InputParameters params = getFactory().getValidParams(variable_type);
  getProblem().addVariable(variable_type, _temperature_name, params);
}
