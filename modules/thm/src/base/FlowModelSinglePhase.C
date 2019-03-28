#include "FlowModelSinglePhase.h"
#include "FlowChannelBase.h"

const std::string FlowModelSinglePhase::DENSITY = "rho";
const std::string FlowModelSinglePhase::FRICTION_FACTOR_DARCY = "f_D";
const std::string FlowModelSinglePhase::DYNAMIC_VISCOSITY = "mu";
const std::string FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL = "Hw";
const std::string FlowModelSinglePhase::MOMENTUM_DENSITY = "rhou";
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
const std::string FlowModelSinglePhase::TOTAL_ENERGY_DENSITY = "rhoE";
const std::string FlowModelSinglePhase::VELOCITY = "vel";

template <>
InputParameters
validParams<FlowModelSinglePhase>()
{
  InputParameters params = validParams<FlowModel>();
  params.addParam<std::string>(
      "suffix", "", "The suffix of the single phase flow model being added.");
  return params;
}

registerMooseObject("THMApp", FlowModelSinglePhase);

FlowModelSinglePhase::FlowModelSinglePhase(const InputParameters & params)
  : FlowModel(params),
    _raw_suffix(params.get<std::string>("suffix")),
    _suffix(_raw_suffix == "" ? "" : "_" + _raw_suffix),
    _rhoA_name(RHOA + _suffix),
    _rhouA_name(RHOUA + _suffix),
    _rhoEA_name(RHOEA + _suffix),
    _density_name(DENSITY + _suffix),
    _total_energy_density_name(TOTAL_ENERGY_DENSITY + _suffix),
    _momentum_density_name(MOMENTUM_DENSITY + _suffix),
    _velocity_name(VELOCITY + _suffix),
    _pressure_name(PRESSURE + _suffix),
    _specific_volume_name(SPECIFIC_VOLUME + _suffix),
    _specific_internal_energy_name(SPECIFIC_INTERNAL_ENERGY + _suffix),
    _temperature_name(TEMPERATURE + _suffix),
    _specific_total_enthalpy_name(SPECIFIC_TOTAL_ENTHALPY + _suffix)
{
  _solution_vars = {_rhoA_name, _rhouA_name, _rhoEA_name};
  _derivative_vars = _solution_vars;
}

void
FlowModelSinglePhase::init()
{
}

void
FlowModelSinglePhase::addVariables()
{
  FlowModel::addCommonVariables();

  unsigned int subdomain_id = _flow_channel.getSubdomainID();
  std::vector<Real> scaling_factor = _sim.getParam<std::vector<Real>>("scaling_factor_1phase");

  // Nonlinear variables
  _sim.addVariable(true, _rhoA_name, _fe_type, subdomain_id, scaling_factor[0]);
  _sim.addVariable(true, _rhouA_name, _fe_type, subdomain_id, scaling_factor[1]);
  _sim.addVariable(true, _rhoEA_name, _fe_type, subdomain_id, scaling_factor[2]);

  // Auxiliary
  _sim.addVariable(false, _density_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _momentum_density_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _total_energy_density_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _velocity_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _pressure_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _specific_volume_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _specific_internal_energy_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _temperature_name, _fe_type, subdomain_id);
  _sim.addVariable(false, _specific_total_enthalpy_name, _fe_type, subdomain_id);
}

void
FlowModelSinglePhase::addInitialConditions()
{
  FlowModel::addCommonInitialConditions();

  bool ics_set = _flow_channel.isParamValid("initial_p") &&
                 _flow_channel.isParamValid("initial_T") &&
                 _flow_channel.isParamValid("initial_vel");

  if (ics_set)
  {
    const std::vector<SubdomainName> & block = _flow_channel.getSubdomainNames();

    const FunctionName & p_fn = getVariableFn("initial_p");
    _sim.addFunctionIC(_pressure_name, p_fn, block);

    const FunctionName & T_fn = getVariableFn("initial_T");
    _sim.addFunctionIC(_temperature_name, T_fn, block);

    const FunctionName & v_fn = getVariableFn("initial_vel");
    _sim.addFunctionIC(_velocity_name, v_fn, block);

    {
      std::string class_name = "RhoFromPressureTemperatureIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _density_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {_pressure_name};
      params.set<std::vector<VariableName>>("T") = {_temperature_name};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rho_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _momentum_density_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {_density_name, _velocity_name};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rhou_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "RhoEFromPressureTemperatureVelocityIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _total_energy_density_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {_pressure_name};
      params.set<std::vector<VariableName>>("T") = {_temperature_name};
      params.set<std::vector<VariableName>>("vel") = {_velocity_name};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rhoE_ic", _raw_suffix), params);
    }

    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _rhoA_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {_density_name, AREA};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rhoA_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _rhouA_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {_momentum_density_name, AREA};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rhouA_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _rhoEA_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {_total_energy_density_name, AREA};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "rhoEA_ic", _raw_suffix), params);
    }

    {
      std::string class_name = "SpecificVolumeIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _specific_volume_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
      params.set<std::vector<VariableName>>("A") = {AREA};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "v_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "SpecificInternalEnergyIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _specific_internal_energy_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
      params.set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
      params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "u_ic", _raw_suffix), params);
    }
    {
      std::string class_name = "SpecificTotalEnthalpyIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = _specific_total_enthalpy_name;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {_pressure_name};
      params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
      params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
      params.set<std::vector<VariableName>>("A") = {AREA};
      _sim.addInitialCondition(
          class_name, Component::genName(_comp_name, "H_ic", _raw_suffix), params);
    }
  }
}

void
FlowModelSinglePhase::addMooseObjects()
{
  FlowModel::addCommonMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // numerical flux user object
  {
    const std::string class_name = "NumericalFlux3EqnHLLC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, _numerical_flux_name, params);
  }

  if (_spatial_discretization == rDG)
    addRDGMooseObjects();

  {
    std::string class_name = "FluidProperties3EqnMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addMaterial(class_name, Component::genName(_comp_name, "fp_mat"), params);
  }

  ////////////////////////////////////////////////////////
  // Adding kernels
  ////////////////////////////////////////////////////////

  // Density equation (transient term + advection term)
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_ie", _raw_suffix), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMassFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<MaterialPropertyName>("vel") = _velocity_name;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_if", _raw_suffix), params);
  }

  // Momentum equation, for 1-D flow channel, x-momentum equation only
  // (transient term + remaining terms[advection, pressure, body force, etc])
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ie", _raw_suffix), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMomentumFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();

    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("arhoEA") = {_rhoEA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<MaterialPropertyName>("vel") = _velocity_name;
    params.set<MaterialPropertyName>("p") = _pressure_name;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_if", _raw_suffix), params);
  }
  {
    std::string class_name = "OneDMomentumAreaGradient";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    // Since rDG uses this kernel and rDG usually uses elemental area by default,
    // a linear area variable must be used specifically.
    params.set<std::vector<VariableName>>("A") = {_A_linear_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("p") = _pressure_name;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<std::vector<VariableName>>("arhoEA") = {_rhoEA_name};
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ps", _raw_suffix), params);
  }
  {
    std::string class_name = "OneDMomentumFriction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<MaterialPropertyName>("D_h") = HYDRAULIC_DIAMETER;
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("arhoEA") = {_rhoEA_name};
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<MaterialPropertyName>("vel") = _velocity_name;
    params.set<MaterialPropertyName>("f_D") = FRICTION_FACTOR_DARCY;
    params.set<MaterialPropertyName>("2phase_multiplier") = UNITY;
    _sim.addKernel(
        class_name, Component::genName(_comp_name, "rhou_friction", _raw_suffix), params);
  }
  {
    std::string class_name = "OneDMomentumGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_gravity", _raw_suffix), params);
  }

  // Total energy equation
  // (transient term + remaining terms[advection, wall heating, work from body force, etc])
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoEA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_ie", _raw_suffix), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDEnergyFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoEA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("arhoEA") = {_rhoEA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<MaterialPropertyName>("vel") = _velocity_name;
    params.set<MaterialPropertyName>("e") = _specific_internal_energy_name;
    params.set<MaterialPropertyName>("p") = _pressure_name;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_if", _raw_suffix), params);
  }
  {
    std::string class_name = "OneDEnergyGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoEA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("arhouA") = {_rhouA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = _density_name;
    params.set<MaterialPropertyName>("vel") = _velocity_name;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_gravity", _raw_suffix), params);
  }

  //////////////////////////////////////
  // Adding auxiliary kernels
  //////////////////////////////////////

  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = EXEC_TIMESTEP_BEGIN;

  // Velocity auxiliary kernel
  {
    std::string class_name = "QuotientAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _velocity_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {_rhouA_name};
    params.set<std::vector<VariableName>>("denominator") = {_rhoA_name};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "vel", _raw_suffix), params);
  }

  {
    // Computes rho = (rho*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = _density_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {_rhoA_name};
    params.set<std::vector<VariableName>>("denominator") = {AREA};
    _sim.addAuxKernel(
        "QuotientAux", Component::genName(_comp_name, "rho_auxkernel", _raw_suffix), params);
  }
  {
    // Computes rhou = (rho*u*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = _momentum_density_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {_rhouA_name};
    params.set<std::vector<VariableName>>("denominator") = {AREA};
    _sim.addAuxKernel(
        "QuotientAux", Component::genName(_comp_name, "rhou_auxkernel", _raw_suffix), params);
  }

  {
    std::string class_name = "THMSpecificVolumeAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _specific_volume_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("A") = {AREA};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "v_aux", _raw_suffix), params);
  }
  {
    std::string class_name = "THMSpecificInternalEnergyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _specific_internal_energy_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "e_aux", _raw_suffix), params);
  }

  {
    std::string class_name = "PressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _pressure_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {_specific_internal_energy_name};
    params.set<std::vector<VariableName>>("v") = {_specific_volume_name};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(
        class_name, Component::genName(_comp_name, "pressure_uv_auxkernel", _raw_suffix), params);
  }
  {
    std::string class_name = "TemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _temperature_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {_specific_internal_energy_name};
    params.set<std::vector<VariableName>>("v") = {_specific_volume_name};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(
        class_name, Component::genName(_comp_name, "T_auxkernel", _raw_suffix), params);
  }

  {
    std::string class_name = "SpecificTotalEnthalpyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _specific_total_enthalpy_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    params.set<std::vector<VariableName>>("p") = {_pressure_name};
    params.set<std::vector<VariableName>>("A") = {AREA};
    _sim.addAuxKernel(
        class_name, Component::genName(_comp_name, "H_auxkernel", _raw_suffix), params);
  }
  {
    // Computes rhoE = (rho*E*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = _total_energy_density_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {_rhoEA_name};
    params.set<std::vector<VariableName>>("denominator") = {AREA};
    _sim.addAuxKernel(
        "QuotientAux", Component::genName(_comp_name, "rhoE_auxkernel", _raw_suffix), params);
  }
}

void
FlowModelSinglePhase::addRDGMooseObjects()
{
  // slope reconstruction material
  {
    const std::string class_name = "RDG3EqnMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
    params.set<std::vector<VariableName>>("A_elem") = {AREA};
    params.set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
    params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addMaterial(class_name, Component::genName(_comp_name, class_name, _raw_suffix), params);
  }

  // advection
  {
    // mass
    const std::string class_name = "NumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _rhoA_name;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
    params.set<std::vector<VariableName>>("rhoA") = {_rhoA_name};
    params.set<std::vector<VariableName>>("rhouA") = {_rhouA_name};
    params.set<std::vector<VariableName>>("rhoEA") = {_rhoEA_name};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addDGKernel(
        class_name, Component::genName(_comp_name, "mass_advection", _raw_suffix), params);

    // momentum
    params.set<NonlinearVariableName>("variable") = _rhouA_name;
    _sim.addDGKernel(
        class_name, Component::genName(_comp_name, "momentum_advection", _raw_suffix), params);

    // energy
    params.set<NonlinearVariableName>("variable") = _rhoEA_name;
    _sim.addDGKernel(
        class_name, Component::genName(_comp_name, "energy_advection", _raw_suffix), params);
  }
}
