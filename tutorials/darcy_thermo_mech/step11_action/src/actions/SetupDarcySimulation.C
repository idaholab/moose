//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupDarcySimulation.h"

// MOOSE includes
#include "FEProblem.h"
#include "AuxiliarySystem.h"

// libMesh includes
#include "libmesh/fe.h"

registerMooseAction("DarcyThermoMechApp", SetupDarcySimulation, "add_aux_variable");
registerMooseAction("DarcyThermoMechApp", SetupDarcySimulation, "add_aux_kernel");
registerMooseAction("DarcyThermoMechApp", SetupDarcySimulation, "add_kernel");

InputParameters
SetupDarcySimulation::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<VariableName>("pressure", "pressure", "The pressure variable.");
  params.addParam<VariableName>("temperature", "temperature", "The temperature variable.");
  params.addParam<bool>(
      "compute_velocity", true, "Enable the auxiliary calculation of velocity from pressure.");
  params.addParam<bool>("compute_pressure", true, "Enable the computation of pressure.");
  params.addParam<bool>("compute_temperature", true, "Enable the computation of temperature.");
  return params;
}

SetupDarcySimulation::SetupDarcySimulation(const InputParameters & parameters)
  : Action(parameters),
    _compute_velocity(getParam<bool>("compute_velocity")),
    _compute_pressure(getParam<bool>("compute_pressure")),
    _compute_temperature(getParam<bool>("compute_temperature"))
{
}

void
SetupDarcySimulation::act()
{
  // Actual names of input variables
  const std::string pressure = getParam<VariableName>("pressure");
  const std::string temperature = getParam<VariableName>("temperature");

  // Velocity AuxVariables
  if (_compute_velocity && _current_task == "add_aux_variable")
  {
    InputParameters var_params = _factory.getValidParams("VectorMooseVariable");
    var_params.set<MooseEnum>("family") = "MONOMIAL_VEC";
    var_params.set<MooseEnum>("order") = "CONSTANT";

    _problem->addAuxVariable("VectorMooseVariable", "velocity", var_params);
  }

  // Velocity AuxKernels
  else if (_compute_velocity && _current_task == "add_aux_kernel")
  {
    InputParameters params = _factory.getValidParams("DarcyVelocity");
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
    params.set<std::vector<VariableName>>("pressure") = {pressure};

    params.set<AuxVariableName>("variable") = "velocity";
    _problem->addAuxKernel("DarcyVelocity", "velocity", params);
  }

  // Kernels
  else if (_current_task == "add_kernel")
  {
    // Flags for aux variables
    const bool is_pressure_aux = _problem->getAuxiliarySystem().hasVariable(pressure);
    const bool is_temperature_aux = _problem->getAuxiliarySystem().hasVariable(temperature);

    // Pressure
    if (_compute_pressure && !is_pressure_aux)
    {
      InputParameters params = _factory.getValidParams("DarcyPressure");
      params.set<NonlinearVariableName>("variable") = pressure;
      _problem->addKernel("DarcyPressure", "darcy_pressure", params);
    }

    // Temperature
    if (_compute_temperature && !is_temperature_aux)
    {
      {
        InputParameters params = _factory.getValidParams("ADHeatConduction");
        params.set<NonlinearVariableName>("variable") = temperature;
        _problem->addKernel("ADHeatConduction", "heat_conduction", params);
      }

      {
        InputParameters params = _factory.getValidParams("ADHeatConductionTimeDerivative");
        params.set<NonlinearVariableName>("variable") = temperature;
        _problem->addKernel("ADHeatConductionTimeDerivative", "heat_conduction_time", params);
      }

      {
        InputParameters params = _factory.getValidParams("DarcyAdvection");
        params.set<NonlinearVariableName>("variable") = temperature;
        params.set<std::vector<VariableName>>("pressure") = {pressure};
        _problem->addKernel("DarcyAdvection", "heat_advection", params);
      }
    }
  }
}
