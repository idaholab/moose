//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelSinglePhase.h"
#include "FlowChannelBase.h"
#include "THMNames.h"

const std::string FlowModelSinglePhase::DENSITY = THM::DENSITY;
const std::string FlowModelSinglePhase::FRICTION_FACTOR_DARCY = THM::FRICTION_FACTOR_DARCY;
const std::string FlowModelSinglePhase::DYNAMIC_VISCOSITY = THM::DYNAMIC_VISCOSITY;
const std::string FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL =
    THM::HEAT_TRANSFER_COEFFICIENT_WALL;
const std::string FlowModelSinglePhase::HYDRAULIC_DIAMETER = THM::HYDRAULIC_DIAMETER;
const std::string FlowModelSinglePhase::PRESSURE = THM::PRESSURE;
const std::string FlowModelSinglePhase::RHOA = THM::RHOA;
const std::string FlowModelSinglePhase::RHOEA = THM::RHOEA;
const std::string FlowModelSinglePhase::RHOUA = THM::RHOUA;
const std::string FlowModelSinglePhase::SOUND_SPEED = THM::SOUND_SPEED;
const std::string FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE =
    THM::SPECIFIC_HEAT_CONSTANT_PRESSURE;
const std::string FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_VOLUME =
    THM::SPECIFIC_HEAT_CONSTANT_VOLUME;
const std::string FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY = THM::SPECIFIC_INTERNAL_ENERGY;
const std::string FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY = THM::SPECIFIC_TOTAL_ENTHALPY;
const std::string FlowModelSinglePhase::SPECIFIC_VOLUME = THM::SPECIFIC_VOLUME;
const std::string FlowModelSinglePhase::TEMPERATURE = THM::TEMPERATURE;
const std::string FlowModelSinglePhase::THERMAL_CONDUCTIVITY = THM::THERMAL_CONDUCTIVITY;
const std::string FlowModelSinglePhase::VELOCITY = THM::VELOCITY;
const std::string FlowModelSinglePhase::VELOCITY_X = THM::VELOCITY_X;
const std::string FlowModelSinglePhase::VELOCITY_Y = THM::VELOCITY_Y;
const std::string FlowModelSinglePhase::VELOCITY_Z = THM::VELOCITY_Z;
const std::string FlowModelSinglePhase::REYNOLDS_NUMBER = THM::REYNOLDS_NUMBER;

InputParameters
FlowModelSinglePhase::validParams()
{
  InputParameters params = FlowModel1PhaseBase::validParams();

  MooseEnum wave_speed_formulation("einfeldt davis", "einfeldt");
  params.addParam<MooseEnum>(
      "wave_speed_formulation", wave_speed_formulation, "Method for computing wave speeds");

  params.addRequiredParam<std::vector<Real>>(
      "scaling_factor_1phase",
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");

  return params;
}

registerMooseObject("ThermalHydraulicsApp", FlowModelSinglePhase);

FlowModelSinglePhase::FlowModelSinglePhase(const InputParameters & params)
  : FlowModel1PhaseBase(params),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factor_1phase"))
{
  // create passive transport solution variables, if any
  const auto & passives_names = _flow_channel.getParam<std::vector<VariableName>>("passives_names");
  _passives_times_area_names.resize(passives_names.size());
  for (const auto i : index_range(passives_names))
    _passives_times_area_names[i] = passives_names[i] + "_times_area";
}

Real
FlowModelSinglePhase::getScalingFactorRhoA() const
{
  return _scaling_factors[0];
}

Real
FlowModelSinglePhase::getScalingFactorRhoUA() const
{
  return _scaling_factors[1];
}

Real
FlowModelSinglePhase::getScalingFactorRhoEA() const
{
  return _scaling_factors[2];
}

void
FlowModelSinglePhase::addVariables()
{
  FlowModel1PhaseBase::addVariables();

  // Add passive transport variables
  const auto scaling_factor_passives =
      _flow_channel.isParamSetByUser("scaling_factor_passives")
          ? _flow_channel.getParam<std::vector<Real>>("scaling_factor_passives")
          : std::vector<Real>(_passives_times_area_names.size(), 1.0);
  mooseAssert(scaling_factor_passives.size() == _passives_times_area_names.size(),
              "'scaling_factor_passives' size must match number of passives.");
  for (const auto i : index_range(_passives_times_area_names))
    _sim.addSimVariable(true,
                        _passives_times_area_names[i],
                        _fe_type,
                        _flow_channel.getSubdomainNames(),
                        scaling_factor_passives[i]);
}

std::vector<VariableName>
FlowModelSinglePhase::solutionVariableNames() const
{
  std::vector<VariableName> vars = {RHOA, RHOUA, RHOEA};
  vars.insert(vars.end(), _passives_times_area_names.begin(), _passives_times_area_names.end());
  return vars;
}

void
FlowModelSinglePhase::addInitialConditions()
{
  FlowModel1PhaseBase::addInitialConditions();

  // passive transport variables
  const auto & passives_ic_fn_names =
      _flow_channel.getParam<std::vector<FunctionName>>("initial_passives");
  for (const auto i : index_range(_passives_times_area_names))
    addPassiveTransportIC(_passives_times_area_names[i], passives_ic_fn_names[i]);
}

void
FlowModelSinglePhase::addKernels()
{
  FlowModel1PhaseBase::addKernels();

  // time derivatives for passive transport variables
  for (const auto & var : _passives_times_area_names)
    addTimeDerivativeKernelIfTransient(var);
}

void
FlowModelSinglePhase::addPassiveTransportIC(const VariableName & var, const FunctionName & ic_fn)
{
  const std::string class_name = "VariableFunctionProductIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<FunctionName>("fn") = ic_fn;
  params.set<std::vector<VariableName>>("var") = {THM::AREA};
  _sim.addSimInitialCondition(class_name, genName(_comp_name, var + "_ic"), params);
}

void
FlowModelSinglePhase::addRhoEAIC()
{
  const std::string class_name = "RhoEAFromPressureTemperatureFunctionVelocityIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = RHOEA;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("p") = {PRESSURE};
  params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
  params.set<FunctionName>("vel") = getVariableFn("initial_vel");
  params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
  params.set<UserObjectName>("fp") = _fp_name;
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoEA_ic"), params);
}

void
FlowModelSinglePhase::addDensityIC()
{
  const std::string class_name = "RhoFromPressureTemperatureIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = DENSITY;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("p") = {PRESSURE};
  params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
  params.set<UserObjectName>("fp") = _fp_name;
  _sim.addSimInitialCondition(class_name, genName(_comp_name, "rho_ic"), params);
}

void
FlowModelSinglePhase::addPressureAux()
{
  const std::string class_name = "PressureAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = PRESSURE;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
  params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
  params.set<UserObjectName>("fp") = _fp_name;
  _sim.addAuxKernel(class_name, genName(_comp_name, "pressure_uv_auxkernel"), params);
}

void
FlowModelSinglePhase::addTemperatureAux()
{
  const std::string class_name = "TemperatureAux";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<AuxVariableName>("variable") = TEMPERATURE;
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
  params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
  params.set<UserObjectName>("fp") = _fp_name;
  _sim.addAuxKernel(class_name, genName(_comp_name, "T_auxkernel"), params);
}

void
FlowModelSinglePhase::addFluidPropertiesMaterials()
{
  {
    const std::string class_name = "ADFluidProperties3EqnMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addMaterial(class_name, genName(_comp_name, "fp_mat"), params);
  }
  {
    const std::string class_name = "ADDynamicViscosityMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<UserObjectName>("fp_1phase") = _fp_name;
    params.set<MaterialPropertyName>("mu") = {DYNAMIC_VISCOSITY};
    params.set<MaterialPropertyName>("v") = {SPECIFIC_VOLUME};
    params.set<MaterialPropertyName>("e") = {SPECIFIC_INTERNAL_ENERGY};
    _sim.addMaterial(class_name, genName(_comp_name, "mu_mat"), params);
  }
}

void
FlowModelSinglePhase::addNumericalFluxUserObject()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.applySpecificParameters(parameters(), {"wave_speed_formulation"});
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<MooseEnum>("emit_on_nan") = "none";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  _sim.addUserObject(class_name, _numerical_flux_name, params);
}

void
FlowModelSinglePhase::addSlopeReconstructionMaterial()
{
  const std::string class_name = "ADRDG3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
  params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
  params.set<std::vector<VariableName>>("A_elem") = {AREA};
  params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
  params.set<std::vector<VariableName>>("passives_times_area") = _passives_times_area_names;
  params.set<MaterialPropertyName>("direction") = DIRECTION;
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
  _sim.addMaterial(class_name, genName(_comp_name, "rdg_3egn_mat"), params);
}

void
FlowModelSinglePhase::addRDGAdvectionDGKernels()
{
  for (const auto & var : solutionVariableNames())
  {
    const std::string class_name = "ADNumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<std::vector<VariableName>>("passives_times_area") = _passives_times_area_names;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addDGKernel(class_name, genName(_comp_name, "flux_kernel_" + var), params);
  }
}
