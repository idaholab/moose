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

registerMooseAction("DarcyThermoMechApp", SetupDarcySimulation, "setup_darcy");

template <>
InputParameters
validParams<SetupDarcySimulation>()
{
  InputParameters params = validParams<Action>();
  params.addParam<VariableName>("pressure", "pressure", "The pressure variable.");
  params.addParam<VariableName>("temperature", "temperature", "The temperature variable.");
  params.addParam<bool>(
      "compute_velocity", true, "Enable the auxiliary calculation of velocity from pressure.");
  params.addParam<bool>("compute_pressure", true, "Enable the computation of pressure.");
  params.addParam<bool>("compute_temperature", true, "Enable the computation of temperature.");
  return params;
}

SetupDarcySimulation::SetupDarcySimulation(InputParameters parameters)
  : Action(parameters),
    _compute_velocity(getParam<bool>("compute_velocity")),
    _compute_pressure(getParam<bool>("compute_pressure")),
    _compute_temperature(getParam<bool>("compute_temperature"))
{
}

void
SetupDarcySimulation::act()
{
  // Helper variables
  const std::string vel_x = "velocity_x";
  const std::string vel_y = "velocity_y";
  const std::string vel_z = "velocity_z";

  // Actual names of input variables
  const std::string pressure = getParam<VariableName>("pressure");
  const std::string temperature = getParam<VariableName>("temperature");

  // Velocity AuxVariables
  if (_compute_velocity && _current_task == "add_aux_variable")
  {
    FEType fe_type(CONSTANT, MONOMIAL);
    _problem->addAuxVariable(vel_x, fe_type);
    _problem->addAuxVariable(vel_y, fe_type);
    _problem->addAuxVariable(vel_z, fe_type);
  }

  // Velocity AuxKernels
  else if (_compute_velocity && _current_task == "add_aux_kernel")
  {
    InputParameters params = _factory.getValidParams("DarcyVelocity");
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
    params.set<std::vector<VariableName>>("pressure") = {pressure};

    params.set<MooseEnum>("component") = "x";
    params.set<AuxVariableName>("variable") = vel_x;
    _problem->addAuxKernel("DarcyVelocity", vel_x, params);

    params.set<MooseEnum>("component") = "y";
    params.set<AuxVariableName>("variable") = vel_y;
    _problem->addAuxKernel("DarcyVelocity", vel_y, params);

    params.set<MooseEnum>("component") = "z";
    params.set<AuxVariableName>("variable") = vel_z;
    _problem->addAuxKernel("DarcyVelocity", vel_z, params);
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
      InputParameters params = _factory.getADValidParams("DarcyPressure");
      params.set<NonlinearVariableName>("variable") = pressure;
      _problem->addADKernel("DarcyPressure", "darcy_pressure", params);
    }

    // Temperature
    if (_compute_temperature && !is_temperature_aux)
    {
      {
        InputParameters params = _factory.getADValidParams("ADHeatConduction");
        params.set<NonlinearVariableName>("variable") = temperature;
        _problem->addADKernel("ADHeatConduction", "heat_conduction", params);
      }

      {
        InputParameters params = _factory.getADValidParams("ADHeatConductionTimeDerivative");
        params.set<NonlinearVariableName>("variable") = temperature;
        _problem->addADKernel("ADHeatConductionTimeDerivative", "heat_conduction_time", params);
      }

      {
        InputParameters params = _factory.getADValidParams("DarcyAdvection");
        params.set<NonlinearVariableName>("variable") = temperature;
        params.set<std::vector<VariableName>>("pressure") = {pressure};
        _problem->addADKernel("DarcyAdvection", "heat_advection", params);
      }
    }
  }
}
