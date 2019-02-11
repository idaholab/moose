#include "FlowModelSinglePhase.h"
#include "PipeBase.h"

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
  return params;
}

registerMooseObject("RELAP7App", FlowModelSinglePhase);

FlowModelSinglePhase::FlowModelSinglePhase(const InputParameters & params) : FlowModel(params) {}

void
FlowModelSinglePhase::init()
{
}

void
FlowModelSinglePhase::addVariables()
{
  FlowModel::addCommonVariables();

  unsigned int subdomain_id = _pipe.getSubdomainID();
  std::vector<Real> scaling_factor = _sim.getParam<std::vector<Real>>("scaling_factor_1phase");

  // Nonlinear variables
  _sim.addVariable(true, RHOA, _fe_type, subdomain_id, scaling_factor[0]);
  _sim.addVariable(true, RHOUA, _fe_type, subdomain_id, scaling_factor[1]);
  _sim.addVariable(true, RHOEA, _fe_type, subdomain_id, scaling_factor[2]);

  _solution_vars = {RHOA, RHOUA, RHOEA};
  _derivative_vars = _solution_vars;

  // Auxiliary
  _sim.addVariable(false, DENSITY, _fe_type, subdomain_id);
  _sim.addVariable(false, MOMENTUM_DENSITY, _fe_type, subdomain_id);
  _sim.addVariable(false, TOTAL_ENERGY_DENSITY, _fe_type, subdomain_id);
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
  bool ics_set = _pipe.isParamValid("initial_p") && _pipe.isParamValid("initial_T") &&
                 _pipe.isParamValid("initial_vel");

  if (ics_set)
  {
    const std::vector<SubdomainName> & block = _pipe.getSubdomainNames();

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
      params.set<VariableName>("variable") = MOMENTUM_DENSITY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {DENSITY, VELOCITY};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhou_ic"), params);
    }
    {
      std::string class_name = "RhoEFromPressureTemperatureVelocityIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = TOTAL_ENERGY_DENSITY;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("p") = {PRESSURE};
      params.set<std::vector<VariableName>>("T") = {TEMPERATURE};
      params.set<std::vector<VariableName>>("vel") = {VELOCITY};
      params.set<UserObjectName>("fp") = _fp_name;
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhoE_ic"), params);
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
      params.set<std::vector<VariableName>>("values") = {MOMENTUM_DENSITY, FlowModel::AREA};
      _sim.addInitialCondition(class_name, Component::genName(_comp_name, "rhouA_ic"), params);
    }
    {
      std::string class_name = "VariableProductIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<SubdomainName>>("block") = block;
      params.set<std::vector<VariableName>>("values") = {TOTAL_ENERGY_DENSITY, FlowModel::AREA};
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

  // coupling vectors
  std::vector<VariableName> cv_rho(1, DENSITY);
  std::vector<VariableName> cv_rhou(1, MOMENTUM_DENSITY);
  std::vector<VariableName> cv_rhoE(1, TOTAL_ENERGY_DENSITY);
  std::vector<VariableName> cv_vel(1, VELOCITY);
  std::vector<VariableName> cv_pressure(1, PRESSURE);
  std::vector<VariableName> cv_enthalpy(1, SPECIFIC_TOTAL_ENTHALPY);
  std::vector<VariableName> cv_temperature(1, TEMPERATURE);
  std::vector<VariableName> cv_v(1, SPECIFIC_VOLUME);
  std::vector<VariableName> cv_internal_energy(1, SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_P_hf(1, HEAT_FLUX_PERIMETER);

  std::vector<VariableName> cv_rhoA(1, RHOA);
  std::vector<VariableName> cv_rhouA(1, RHOUA);
  std::vector<VariableName> cv_rhoEA(1, RHOEA);
  std::vector<VariableName> cv_area(1, AREA);
  std::vector<VariableName> cv_D_h(1, HYDRAULIC_DIAMETER);

  ////////////////////////////////////////////////////////
  // Adding kernels
  ////////////////////////////////////////////////////////

  // Density equation (transient term + advection term)
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMassFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<MaterialPropertyName>("rho") = DENSITY;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rho_if"), params);
  }

  // Momentum equation, for 1-D pipe, x-momentum equation only
  // (transient term + remaining terms[advection, pressure, body force, etc])
  {
    std::string class_name = "TimeDerivative";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDMomentumFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();

    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    // Since rDG uses this kernel and rDG usually uses elemental area by default,
    // a linear area variable must be used specifically.
    params.set<std::vector<VariableName>>("A") = {_A_linear_name};
    params.set<MaterialPropertyName>("direction") = DIRECTION;
    params.set<MaterialPropertyName>("p") = PRESSURE;
    params.set<MaterialPropertyName>("alpha") = UNITY;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhou_ps"), params);
  }
  {
    std::string class_name = "OneDMomentumFriction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("D_h") = {HYDRAULIC_DIAMETER};
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    if (_lump_mass_matrix)
      params.set<bool>("lumping") = true;
    _sim.addKernel(class_name, Component::genName(_comp_name, "rhoE_ie"), params);
  }
  if (_spatial_discretization == CG)
  {
    std::string class_name = "OneDEnergyFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = cv_rhou;
    params.set<std::vector<VariableName>>("denominator") = cv_rho;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "vel"), params);
  }

  {
    // Computes rho = (rho*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = DENSITY;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = cv_rhoA;
    params.set<std::vector<VariableName>>("denominator") = cv_area;
    _sim.addAuxKernel("QuotientAux", Component::genName(_comp_name, "rho_auxkernel"), params);
  }
  {
    // Computes rhou = (rho*u*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = MOMENTUM_DENSITY;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = cv_rhouA;
    params.set<std::vector<VariableName>>("denominator") = cv_area;
    _sim.addAuxKernel("QuotientAux", Component::genName(_comp_name, "rhou_auxkernel"), params);
  }

  {
    std::string class_name = "SpecificVolumeAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_VOLUME;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "v_aux"), params);
  }
  {
    std::string class_name = "SpecificInternalEnergyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_INTERNAL_ENERGY;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "e_aux"), params);
  }

  {
    std::string class_name = "PressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = cv_internal_energy;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "pressure_uv_auxkernel"), params);
  }
  {
    std::string class_name = "TemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = TEMPERATURE;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = cv_internal_energy;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "T_auxkernel"), params);
  }

  {
    std::string class_name = "SpecificTotalEnthalpyAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = SPECIFIC_TOTAL_ENTHALPY;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("p") = cv_pressure;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "H_auxkernel"), params);
  }
  {
    // Computes rhoE = (rho*E*A)/A
    InputParameters params = _factory.getValidParams("QuotientAux");
    params.set<AuxVariableName>("variable") = TOTAL_ENERGY_DENSITY;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("numerator") = cv_rhoEA;
    params.set<std::vector<VariableName>>("denominator") = cv_area;
    _sim.addAuxKernel("QuotientAux", Component::genName(_comp_name, "rhoE_auxkernel"), params);
  }
}

void
FlowModelSinglePhase::addRDGMooseObjects()
{
  // slope reconstruction material
  {
    const std::string class_name = "RDG3EqnMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<MooseEnum>("scheme") = _rdg_slope_reconstruction;
    params.set<std::vector<VariableName>>("A_elem") = {AREA};
    params.set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
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
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {_A_linear_name};
    params.set<std::vector<VariableName>>("A") = {AREA};
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
