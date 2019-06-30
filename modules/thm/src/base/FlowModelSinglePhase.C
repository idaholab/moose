#include "FlowModelSinglePhase.h"
#include "FlowChannelBase.h"

const std::string FlowModelSinglePhase::DENSITY = "rho";
const std::string FlowModelSinglePhase::FRICTION_FACTOR_DARCY = "f_D";
const std::string FlowModelSinglePhase::DYNAMIC_VISCOSITY = "mu";
const std::string FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL = "Hw";
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

template <>
InputParameters
validParams<FlowModelSinglePhase>()
{
  InputParameters params = validParams<FlowModel>();
  return params;
}

registerMooseObject("THMApp", FlowModelSinglePhase);

FlowModelSinglePhase::FlowModelSinglePhase(const InputParameters & params) : FlowModel(params) {}

void
FlowModelSinglePhase::init()
{
}

void
FlowModelSinglePhase::addVariables()
{
  FlowModel::addCommonVariables();

  unsigned int subdomain_id = _flow_channel.getSubdomainID();
  std::vector<Real> scaling_factor = _sim.getParamTempl<std::vector<Real>>("scaling_factor_1phase");

  // Nonlinear variables
  _sim.addVariable(true, RHOA, _fe_type, subdomain_id, scaling_factor[0]);
  _sim.addVariable(true, RHOUA, _fe_type, subdomain_id, scaling_factor[1]);
  _sim.addVariable(true, RHOEA, _fe_type, subdomain_id, scaling_factor[2]);

  _solution_vars = {RHOA, RHOUA, RHOEA};
  _derivative_vars = _solution_vars;

  // Auxiliary
  _sim.addVariable(false, DENSITY, _fe_type, subdomain_id);
  _sim.addVariable(false, VELOCITY, _fe_type, subdomain_id);
  _sim.addVariable(false, PRESSURE, _fe_type, subdomain_id);
  _sim.addVariable(false, SPECIFIC_VOLUME, _fe_type, subdomain_id);
  _sim.addVariable(false, SPECIFIC_INTERNAL_ENERGY, _fe_type, subdomain_id);
  _sim.addVariable(false, TEMPERATURE, _fe_type, subdomain_id);
  _sim.addVariable(false, SPECIFIC_TOTAL_ENTHALPY, _fe_type, subdomain_id);
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

    const FunctionName & v_fn = getVariableFn("initial_vel");
    _sim.addFunctionIC(VELOCITY, v_fn, block);

    {
      std::string class_name = "RhoFromPressureTemperatureIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = DENSITY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {PRESSURE};
      params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rho_ic"), params);
    }

    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = RHOA;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {DENSITY, FlowModel::AREA};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhoA_ic"), params);
    }
    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = RHOUA;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {DENSITY, VELOCITY, FlowModel::AREA};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhouA_ic"), params);
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
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhoEA_ic"), params);
    }

    {
      std::string class_name = "SpecificVolumeIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = SPECIFIC_VOLUME;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "v_ic"), params);
    }
    {
      std::string class_name = "SpecificInternalEnergyIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("rhoA") = {RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "u_ic"), params);
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
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "H_ic"), params);
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
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMassFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_if"), params);
  }

  // Momentum equation, for 1-D flow channel, x-momentum equation only
  // (transient term + remaining terms[advection, pressure, body force, etc])
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMomentumFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();

    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {RHOEA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<MaterialPropertyName>("p") = PRESSURE;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_if"), params);
  }
  {
    std::string class_name = "OneDMomentumAreaGradient";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    if (_spatial_discretization == CG)
      params.set<std::vector<VariableName>>("A") = {AREA};
    else if (_spatial_discretization == rDG)
      params.set<std::vector<VariableName>>("A") = {AREA_LINEAR};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("p") = PRESSURE;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<std::vector<VariableName>>("arhoEA") = {RHOEA};
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ps"), params);
  }
  {
    std::string class_name = "OneDMomentumFriction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<MaterialPropertyName>("D_h") = {HYDRAULIC_DIAMETER};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {RHOEA};
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<MaterialPropertyName>("f_D") = FRICTION_FACTOR_DARCY;
    params.set<MaterialPropertyName>("2phase_multiplier") = UNITY;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_friction"), params);
  }
  {
    std::string class_name = "OneDMomentumGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_gravity"), params);
  }

  // Total energy equation
  // (transient term + remaining terms[advection, wall heating, work from body force, etc])
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDEnergyFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {RHOEA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<MaterialPropertyName>("e") = SPECIFIC_INTERNAL_ENERGY;
    params.set<MaterialPropertyName>("p") = PRESSURE;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_if"), params);
  }
  {
    std::string class_name = "OneDEnergyGravity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("arhoA") = {RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {RHOUA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_gravity"), params);
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
    params.set<AuxVariableName>("variable") = VELOCITY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {RHOUA};
    params.set<std::vector<VariableName>>("denominator") = {RHOA};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "vel"), params);
  }

  {
    // Computes rho = (rho*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = DENSITY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = {RHOA};
    params.set<std::vector<VariableName>>("denominator") = {AREA};
    _sim.addAuxKernel("QuotientAux", Component::genName(_comp_name, "rho_auxkernel"), params);
  }

  {
    std::string class_name = "THMSpecificVolumeAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_VOLUME;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("A") = {AREA};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "v_aux"), params);
  }
  {
    std::string class_name = "THMSpecificInternalEnergyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "e_aux"), params);
  }

  {
    std::string class_name = "PressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "pressure_uv_auxkernel"), params);
  }
  {
    std::string class_name = "TemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = TEMPERATURE;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = {SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {SPECIFIC_VOLUME};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "T_auxkernel"), params);
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
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "H_auxkernel"), params);
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
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addMaterial(class_name, Component::genName(_comp_name, class_name), params);
  }

  // advection
  {
    // mass
    const std::string class_name = "NumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA_LINEAR};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "mass_advection"), params);

    // momentum
    params.set<NonlinearVariableName>("variable") = RHOUA;
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "momentum_advection"), params);

    // energy
    params.set<NonlinearVariableName>("variable") = RHOEA;
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "energy_advection"), params);
  }
}
