//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Shaft.h"
#include "ShaftConnectable.h"
#include "Component1DConnection.h"

registerMooseObject("ThermalHydraulicsApp", Shaft);

InputParameters
Shaft::validParams()
{
  InputParameters params = Component::validParams();
  params.addParam<Real>("scaling_factor_omega", 1.0, "Scaling factor for omega [-]");
  params.addParam<Real>("initial_speed", "Initial shaft speed");
  params.addParam<bool>("ad", true, "Use AD version or not");
  params.addRequiredParam<std::vector<std::string>>("connected_components",
                                                    "Names of the connected components");
  params.addClassDescription("Component that connects torque of turbomachinery components");
  return params;
}

Shaft::Shaft(const InputParameters & parameters)
  : Component(parameters),
    _scaling_factor_omega(getParam<Real>("scaling_factor_omega")),
    _omega_var_name(genName(name(), "omega")),
    _connected_components(getParam<std::vector<std::string>>("connected_components"))
{
  for (auto & comp_name : _connected_components)
    addDependency(comp_name);
}

void
Shaft::init()
{
  Component::init();

  for (const auto & comp_name : _connected_components)
  {
    if (hasComponentByName<Component>(comp_name))
    {
      const Component & c = getComponentByName<Component>(comp_name);
      const ShaftConnectable & scc = dynamic_cast<const ShaftConnectable &>(c);
      scc.setShaftName(name());
    }
  }
}

void
Shaft::check() const
{
  if (_connected_components.size() == 0)
    logError("No components are connected to the shaft.");

  bool ics_set = getTHMProblem().hasInitialConditionsFromFile() || isParamValid("initial_speed");
  if (!ics_set && !_app.isRestarting())
    logError("The `initial_speed` parameter is missing.");
}

void
Shaft::addVariables()
{
  getTHMProblem().addSimVariable(
      true, _omega_var_name, libMesh::FEType(FIRST, SCALAR), _scaling_factor_omega);

  if (isParamValid("initial_speed"))
    getTHMProblem().addConstantScalarIC(_omega_var_name, getParam<Real>("initial_speed"));
}

void
Shaft::addMooseObjects()
{
  std::vector<UserObjectName> uo_names;

  for (const auto & comp_name : _connected_components)
  {
    const Component & c = getComponentByName<Component>(comp_name);
    const ShaftConnectable & scc = dynamic_cast<const ShaftConnectable &>(c);
    uo_names.push_back(scc.getShaftConnectedUserObjectName());
  }

  if (getParam<bool>("ad"))
  {
    {
      std::string class_name = "ADShaftTimeDerivativeScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _omega_var_name;
      params.set<std::vector<UserObjectName>>("uo_names") = {uo_names};
      getTHMProblem().addScalarKernel(class_name, genName(name(), "td"), params);
    }

    for (std::size_t i = 0; i < uo_names.size(); i++)
    {
      std::string class_name = "ADShaftComponentTorqueScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _omega_var_name;
      params.set<UserObjectName>("shaft_connected_component_uo") = uo_names[i];
      getTHMProblem().addScalarKernel(class_name, genName(name(), i, "shaft_speed"), params);
    }
  }
  else
  {
    {
      std::string class_name = "ShaftTimeDerivativeScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _omega_var_name;
      params.set<std::vector<UserObjectName>>("uo_names") = {uo_names};
      getTHMProblem().addScalarKernel(class_name, genName(name(), "td"), params);
    }

    for (std::size_t i = 0; i < uo_names.size(); i++)
    {
      std::string class_name = "ShaftComponentTorqueScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = _omega_var_name;
      params.set<UserObjectName>("shaft_connected_component_uo") = uo_names[i];
      getTHMProblem().addScalarKernel(class_name, genName(name(), i, "shaft_speed"), params);
    }
  }
}

VariableName
Shaft::getOmegaVariableName() const
{
  return _omega_var_name;
}
