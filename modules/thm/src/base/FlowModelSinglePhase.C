#include "FlowModelSinglePhase.h"
#include "Simulation.h"
#include "Factory.h"
#include "PipeBase.h"

const std::string FlowModelSinglePhase::DENSITY = "rho";
const std::string FlowModelSinglePhase::DRAG_COEFFICIENT = "Cw";
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

FlowModelSinglePhase::FlowModelSinglePhase(const std::string & name, const InputParameters & params)
  : FlowModel(name, params)
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
  _sim.addVariable(false, HEAT_TRANSFER_COEFFICIENT_WALL, _fe_type, subdomain_id);
}

void
FlowModelSinglePhase::addMooseObjects()
{
  FlowModel::addCommonMooseObjects();

  if (_spatial_discretization == rDG)
    addRDGMooseObjects();

  const InputParameters & pars = _pipe.parameters();
  const std::string fp_name = pars.get<UserObjectName>("fp");

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
  std::vector<VariableName> cv_HTC(1, HEAT_TRANSFER_COEFFICIENT_WALL);
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
    if (_spatial_discretization == rDG)
      params.set<std::vector<VariableName>>("A") = {AREA + "_linear"};
    else
      params.set<std::vector<VariableName>>("A") = {AREA};
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
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    params.set<MaterialPropertyName>("vel") = VELOCITY;
    params.set<MaterialPropertyName>("Cw") = "Cw";
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
    params.set<UserObjectName>("fp") = fp_name;
    _sim.addAuxKernel(class_name, Component::genName(_comp_name, "pressure_uv_auxkernel"), params);
  }
  {
    std::string class_name = "TemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = TEMPERATURE;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("e") = cv_internal_energy;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<UserObjectName>("fp") = fp_name;
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
  const UserObjectName & fp_name = _pipe.parameters().get<UserObjectName>("fp");
  ExecFlagEnum lin_execute_on(MooseUtils::getDefaultExecFlagEnum());
  lin_execute_on = {EXEC_LINEAR};

  // numerical flux user object
  {
    const std::string class_name = "NumericalFlux3EqnHLLC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = fp_name;
    params.set<ExecFlagEnum>("execute_on") = lin_execute_on;
    _sim.addUserObject(class_name, _numerical_flux_name, params);
  }

  // advection
  {
    // mass
    const std::string class_name = "NumericalFlux3EqnDGKernel";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = RHOA;
    params.set<std::vector<SubdomainName>>("block") = _pipe.getSubdomainNames();
    params.set<std::vector<VariableName>>("A_linear") = {AREA + "_linear"};
    params.set<std::vector<VariableName>>("A") = {AREA};
    params.set<std::vector<VariableName>>("rhoA") = {RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {RHOEA};
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<bool>("implicit") = _implicit_rdg;
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "mass_advection"), params);

    // momentum
    params.set<NonlinearVariableName>("variable") = RHOUA;
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "momentum_advection"), params);

    // energy
    params.set<NonlinearVariableName>("variable") = RHOEA;
    _sim.addDGKernel(class_name, Component::genName(_comp_name, "energy_advection"), params);
  }
}
