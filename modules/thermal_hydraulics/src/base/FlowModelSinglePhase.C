//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelSinglePhase.h"
#include "FlowChannelBase.h"

const std::string FlowModelSinglePhase::DENSITY = "rho";
const std::string FlowModelSinglePhase::FRICTION_FACTOR_DARCY = "f_D";
const std::string FlowModelSinglePhase::DYNAMIC_VISCOSITY = "mu";
const std::string FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL = "Hw";
const std::string FlowModelSinglePhase::HYDRAULIC_DIAMETER = "D_h";
const std::string FlowModelSinglePhase::PRESSURE = "p";
const std::string FlowModelSinglePhase::RHOA = "rhoA";
const std::string FlowModelSinglePhase::RHOEA = "rhoEA";
const std::string FlowModelSinglePhase::RHOUA = "rhouA";
const std::string FlowModelSinglePhase::SOUND_SPEED = "c";
const std::string FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE = "cp";
const std::string FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_VOLUME = "cv";
const std::string FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY = "e";
const std::string FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY = "H";
const std::string FlowModelSinglePhase::SPECIFIC_VOLUME = "v";
const std::string FlowModelSinglePhase::TEMPERATURE = "T";
const std::string FlowModelSinglePhase::THERMAL_CONDUCTIVITY = "k";
const std::string FlowModelSinglePhase::VELOCITY = "vel";
const std::string FlowModelSinglePhase::VELOCITY_X = "vel_x";
const std::string FlowModelSinglePhase::VELOCITY_Y = "vel_y";
const std::string FlowModelSinglePhase::VELOCITY_Z = "vel_z";
const std::string FlowModelSinglePhase::REYNOLDS_NUMBER = "Re";

InputParameters
FlowModelSinglePhase::validParams()
{
  InputParameters params = FlowModel::validParams();
  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");
  params.addRequiredParam<MooseEnum>("rdg_slope_reconstruction",
                                     "Slope reconstruction type for rDG");
  params.addRequiredParam<std::vector<Real>>(
      "scaling_factor_1phase",
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  return params;
}

registerMooseObject("ThermalHydraulicsApp", FlowModelSinglePhase);

FlowModelSinglePhase::FlowModelSinglePhase(const InputParameters & params)
  : FlowModel(params),
    _rdg_slope_reconstruction(params.get<MooseEnum>("rdg_slope_reconstruction")),
    _numerical_flux_name(params.get<UserObjectName>("numerical_flux")),
    _scaling_factors(getParam<std::vector<Real>>("scaling_factor_1phase"))
{
}

void
FlowModelSinglePhase::init()
{
}

void
FlowModelSinglePhase::addVariables()
{
  FlowModel::addCommonVariables();

  const std::vector<SubdomainName> & subdomains = _flow_channel.getSubdomainNames();

  // Nonlinear variables
  _sim.addSimVariable(true, RHOA, _fe_type, subdomains, _scaling_factors[0]);
  _sim.addSimVariable(true, RHOUA, _fe_type, subdomains, _scaling_factors[1]);
  _sim.addSimVariable(true, RHOEA, _fe_type, subdomains, _scaling_factors[2]);

  _solution_vars = {RHOA, RHOUA, RHOEA};
  _derivative_vars = _solution_vars;

  // Auxiliary
  _sim.addSimVariable(false, DENSITY, _fe_type, subdomains);
  if (_output_vector_velocity)
  {
    _sim.addSimVariable(false, VELOCITY_X, _fe_type, subdomains);
    _sim.addSimVariable(false, VELOCITY_Y, _fe_type, subdomains);
    _sim.addSimVariable(false, VELOCITY_Z, _fe_type, subdomains);
  }
  else
    _sim.addSimVariable(false, VELOCITY, _fe_type, subdomains);
  _sim.addSimVariable(false, PRESSURE, _fe_type, subdomains);
  _sim.addSimVariable(false, SPECIFIC_VOLUME, _fe_type, subdomains);
  _sim.addSimVariable(false, SPECIFIC_INTERNAL_ENERGY, _fe_type, subdomains);
  _sim.addSimVariable(false, TEMPERATURE, _fe_type, subdomains);
  _sim.addSimVariable(false, SPECIFIC_TOTAL_ENTHALPY, _fe_type, subdomains);
}

void
FlowModelSinglePhase::addInitialConditions()
{
  FlowModel::addCommonInitialConditions();

  if (_flow_channel.isParamValid("initial_p") && _flow_channel.isParamValid("initial_T") &&
      _flow_channel.isParamValid("initial_vel"))
  {
    const std::vector<SubdomainName> & block = _flow_channel.getSubdomainNames();

    const FunctionName & p_fn = getVariableFn("initial_p");
    _sim.addFunctionIC(PRESSURE, p_fn, block);

    const FunctionName & T_fn = getVariableFn("initial_T");
    _sim.addFunctionIC(TEMPERATURE, T_fn, block);

    const FunctionName & vel_fn = getVariableFn("initial_vel");
    if (_output_vector_velocity)
    {
      std::vector<VariableName> var_name = {VELOCITY_X, VELOCITY_Y, VELOCITY_Z};
      for (const auto i : make_range(Moose::dim))
      {
        std::string class_name = "VectorVelocityIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = var_name[i];
        params.set<FunctionName>("vel_fn") = vel_fn;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<unsigned int>("component") = i;
        _sim.addSimInitialCondition(class_name, genName(_comp_name, "vel_ic", i), params);
      }

      {
        std::string class_name = "VariableFunctionProductIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = RHOUA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("var") = {RHOA};
        params.set<FunctionName>("fn") = vel_fn;
        _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhouA_ic"), params);
      }
      {
        std::string class_name = "RhoEAFromPressureTemperatureFunctionVelocityIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = RHOEA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("p") = {PRESSURE};
        params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
        params.set<FunctionName>("vel") = vel_fn;
        params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
        params.set<UserObjectName>("fp") = _fp_name;
        _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoEA_ic"), params);
      }
    }
    else
    {
      _sim.addFunctionIC(VELOCITY, vel_fn, block);

      {
        std::string class_name = "VariableProductIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = RHOUA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("values") = {DENSITY, VELOCITY, FlowModel::AREA};
        _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhouA_ic"), params);
      }
      {
        std::string class_name = "RhoEAFromPressureTemperatureVelocityIC";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = RHOEA;
        params.set<std::vector<SubdomainName>>("block") = block;
        params.set<std::vector<VariableName>>("p") = {PRESSURE};
        params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
        params.set<std::vector<VariableName>>("vel") = {VELOCITY};
        params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
        params.set<UserObjectName>("fp") = _fp_name;
        _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoEA_ic"), params);
      }
    }

    {
      std::string class_name = "RhoFromPressureTemperatureIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = DENSITY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {PRESSURE};
      params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "rho_ic"), params);
    }

    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = RHOA;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {DENSITY, FlowModel::AREA};
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "rhoA_ic"), params);
    }

    {
      std::string class_name = "SpecificVolumeIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = SPECIFIC_VOLUME;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "v_ic"), params);
    }
    {
      std::string class_name = "SpecificInternalEnergyIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "u_ic"), params);
    }
    {
      std::string class_name = "SpecificTotalEnthalpyIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = SPECIFIC_TOTAL_ENTHALPY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {PRESSURE};
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      _sim.addSimInitialCondition(class_name, genName(_comp_name, "H_ic"), params);
    }
  }
}

void
FlowModelSinglePhase::addMooseObjects()
{
  FlowModel::addCommonMooseObjects();

  addNumericalFluxUserObject();
  addRDGMooseObjects();

  {
    std::string class_name = "ADFluidProperties3EqnMaterial";
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

  ////////////////////////////////////////////////////////
  // Adding kernels
  ////////////////////////////////////////////////////////

  // Density equation (transient term + advection term)
  {
    std::string class_name = "ADTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, genName(_comp_name, "rho_ie"), params);
  }

  // Momentum equation, for 1-D flow channel, x-momentum equation only
  // (transient term + remaining terms[advection, pressure, body force, etc])
  {
    std::string class_name = "ADTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, genName(_comp_name, "rhou_ie"), params);
  }
  {
    std::string class_name = "ADOneD3EqnMomentumAreaGradient";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA_LINEAR};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("p") = PRESSURE;
    _sim.addKernel(class_name, genName(_comp_name, "rhou_ps"), params);
  }
  {
    std::string class_name = "ADOneD3EqnMomentumFriction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<MaterialPropertyName>("D_h") = {HYDRAULIC_DIAMETER};
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<MaterialPropertyName>("f_D") = FRICTION_FACTOR_DARCY;
    _sim.addKernel(class_name, genName(_comp_name, "rhou_friction"), params);
  }
  {
    std::string class_name = "ADOneD3EqnMomentumGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, genName(_comp_name, "rhou_gravity"), params);
  }

  // Total energy equation
  // (transient term + remaining terms[advection, wall heating, work from body force, etc])
  {
    std::string class_name = "ADTimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, genName(_comp_name, "rhoE_ie"), params);
  }
  {
    std::string class_name = "ADOneD3EqnEnergyGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, genName(_comp_name, "rhoE_gravity"), params);
  }

  //////////////////////////////////////
  // Adding auxiliary kernels
  //////////////////////////////////////

  if (_output_vector_velocity)
  {
    // Vector-valued velocity
    {
      ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
      execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};

      std::vector<AuxVariableName> var_names = {VELOCITY_X, VELOCITY_Y, VELOCITY_Z};
      for (const auto i : make_range(Moose::dim))
      {
        std::string class_name = "ADVectorVelocityComponentAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = var_names[i];
        params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
        params.set<std::vector<VariableName>>("arhoA") = {RHOA};
        params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
        params.set<MaterialPropertyName>("direction") = DIRECTION;
        params.set<unsigned int>("component") = i;
        params.set<ExecFlagEnum>("execute_on") = execute_on;
        _sim.addAuxKernel(class_name, genName(_comp_name, i, "vel_vec"), params);
      }
    }
  }
  else
  {
    ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
    execute_on = {EXEC_INITIAL, EXEC_TIMESTEP_END};

    // Velocity auxiliary kernel
    {
      std::string class_name = "QuotientAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = VELOCITY;
      params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
      params.set<std::vector<VariableName>>("numerator") = {RHOUA};
      params.set<std::vector<VariableName>>("denominator") = {RHOA};
      params.set<ExecFlagEnum>("execute_on") = execute_on;
      _sim.addAuxKernel(class_name, genName(_comp_name, "vel"), params);
    }
  }

  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = EXEC_TIMESTEP_BEGIN;

  {
    // Computes rho = (rho*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = DENSITY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {RHOA};
    params.set<std::vector<VariableName>>("denominator") = {AREA};
    _sim.addAuxKernel("QuotientAux", genName(_comp_name, "rho_auxkernel"), params);
  }

  {
    std::string class_name = "THMSpecificVolumeAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_VOLUME;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("A") = {AREA};
    _sim.addAuxKernel(class_name, genName(_comp_name, "v_aux"), params);
  }
  {
    std::string class_name = "THMSpecificInternalEnergyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    _sim.addAuxKernel(class_name, genName(_comp_name, "e_aux"), params);
  }

  {
    std::string class_name = "PressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, genName(_comp_name, "pressure_uv_auxkernel"), params);
  }
  {
    std::string class_name = "TemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = TEMPERATURE;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, genName(_comp_name, "T_auxkernel"), params);
  }

  {
    std::string class_name = "SpecificTotalEnthalpyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_TOTAL_ENTHALPY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<std::vector<VariableName>>("p") = {PRESSURE};
    params.set<std::vector<VariableName>>("A") = {AREA};
    _sim.addAuxKernel(class_name, genName(_comp_name, "H_auxkernel"), params);
  }
}

void
FlowModelSinglePhase::addNumericalFluxUserObject()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  params.set<MooseEnum>("emit_on_nan") = "none";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  _sim.addUserObject(class_name, _numerical_flux_name, params);
}

void
FlowModelSinglePhase::addRDGMooseObjects()
{
  // slope reconstruction material
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
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addMaterial(class_name, genName(_comp_name, "rdg_3egn_mat"), params);
  }

  // advection
  {
    // mass
    const std::string class_name = "ADNumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addDGKernel(class_name, genName(_comp_name, "mass_advection"), params);

    // momentum
    params.set<NonlinearVariableName>("variable") = RHOUA;
    _sim.addDGKernel(class_name, genName(_comp_name, "momentum_advection"), params);

    // energy
    params.set<NonlinearVariableName>("variable") = RHOEA;
    _sim.addDGKernel(class_name, genName(_comp_name, "energy_advection"), params);
  }
}
