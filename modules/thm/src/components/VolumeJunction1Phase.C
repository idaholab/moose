#include "VolumeJunction1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("THMApp", VolumeJunction1Phase);

const unsigned int VolumeJunction1Phase::N_EQ = 5;

template <>
InputParameters
validParams<VolumeJunction1Phase>()
{
  InputParameters params = validParams<VolumeJunctionBase>();

  params.addRequiredParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addRequiredParam<FunctionName>("initial_vel_x", "Initial velocity in x-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_y", "Initial velocity in y-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_z", "Initial velocity in z-direction [m/s]");

  params.addParam<Real>("scaling_factor_rhoV", 1.0, "Scaling factor for rho*V");
  params.addParam<Real>("scaling_factor_rhouV", 1.0, "Scaling factor for rho*u*V");
  params.addParam<Real>("scaling_factor_rhovV", 1.0, "Scaling factor for rho*v*V");
  params.addParam<Real>("scaling_factor_rhowV", 1.0, "Scaling factor for rho*w*V");
  params.addParam<Real>("scaling_factor_rhoEV", 1.0, "Scaling factor for rho*E*V");

  params.addClassDescription("Junction between 1-phase flow channels that has a non-zero volume");

  return params;
}

VolumeJunction1Phase::VolumeJunction1Phase(const InputParameters & params)
  : VolumeJunctionBase(params),

    _scaling_factor_rhoV(getParam<Real>("scaling_factor_rhoV")),
    _scaling_factor_rhouV(getParam<Real>("scaling_factor_rhouV")),
    _scaling_factor_rhovV(getParam<Real>("scaling_factor_rhovV")),
    _scaling_factor_rhowV(getParam<Real>("scaling_factor_rhowV")),
    _scaling_factor_rhoEV(getParam<Real>("scaling_factor_rhoEV")),

    _rhoV_var_name(genName(name(), "rhoV")),
    _rhouV_var_name(genName(name(), "rhouV")),
    _rhovV_var_name(genName(name(), "rhovV")),
    _rhowV_var_name(genName(name(), "rhowV")),
    _rhoEV_var_name(genName(name(), "rhoEV")),
    _pressure_var_name(genName(name(), "p")),
    _temperature_var_name(genName(name(), "T")),
    _velocity_var_name(genName(name(), "vel"))
{
}

void
VolumeJunction1Phase::check() const
{
  VolumeJunctionBase::check();

  if (_flow_model_id != THM::FM_SINGLE_PHASE)
    logModelNotImplementedError(_flow_model_id);
  if (_spatial_discretization != FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
VolumeJunction1Phase::addVariables()
{
  auto connected_subdomains = getConnectedSubdomainNames();

  Function & initial_p_fn = _sim.getFunction(getParam<FunctionName>("initial_p"));
  Function & initial_T_fn = _sim.getFunction(getParam<FunctionName>("initial_T"));
  Function & initial_vel_x_fn = _sim.getFunction(getParam<FunctionName>("initial_vel_x"));
  Function & initial_vel_y_fn = _sim.getFunction(getParam<FunctionName>("initial_vel_y"));
  Function & initial_vel_z_fn = _sim.getFunction(getParam<FunctionName>("initial_vel_z"));

  initial_p_fn.initialSetup();
  initial_T_fn.initialSetup();
  initial_vel_x_fn.initialSetup();
  initial_vel_y_fn.initialSetup();
  initial_vel_z_fn.initialSetup();

  const Real initial_p = initial_p_fn.value(0, _position);
  const Real initial_T = initial_T_fn.value(0, _position);
  const Real initial_vel_x = initial_vel_x_fn.value(0, _position);
  const Real initial_vel_y = initial_vel_y_fn.value(0, _position);
  const Real initial_vel_z = initial_vel_z_fn.value(0, _position);

  const SinglePhaseFluidProperties & fp =
      _sim.getUserObjectTempl<SinglePhaseFluidProperties>(_fp_name);
  const Real initial_rho = fp.rho_from_p_T(initial_p, initial_T);
  const RealVectorValue vel(initial_vel_x, initial_vel_y, initial_vel_z);
  const Real initial_E = fp.e_from_p_rho(initial_p, initial_rho) + 0.5 * vel * vel;

  _sim.addVariable(
      true, _rhoV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhoV);
  _sim.addConstantScalarIC(_rhoV_var_name, initial_rho * _volume);
  _sim.addVariable(
      true, _rhouV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhouV);
  _sim.addConstantScalarIC(_rhouV_var_name, initial_rho * initial_vel_x * _volume);
  _sim.addVariable(
      true, _rhovV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhovV);
  _sim.addConstantScalarIC(_rhovV_var_name, initial_rho * initial_vel_y * _volume);
  _sim.addVariable(
      true, _rhowV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhowV);
  _sim.addConstantScalarIC(_rhowV_var_name, initial_rho * initial_vel_z * _volume);
  _sim.addVariable(
      true, _rhoEV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhoEV);
  _sim.addConstantScalarIC(_rhoEV_var_name, initial_rho * initial_E * _volume);

  _sim.addVariable(false, _pressure_var_name, FEType(FIRST, SCALAR), connected_subdomains);
  _sim.addConstantScalarIC(_pressure_var_name, initial_p);
  _sim.addVariable(false, _temperature_var_name, FEType(FIRST, SCALAR), connected_subdomains);
  _sim.addConstantScalarIC(_temperature_var_name, initial_T);
  _sim.addVariable(false, _velocity_var_name, FEType(FIRST, SCALAR), connected_subdomains);
  _sim.addConstantScalarIC(_velocity_var_name, vel.norm());
}

void
VolumeJunction1Phase::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // Add user object for computing and storing the fluxes
  const std::string volume_junction_uo_name = genName(name(), "volume_junction_uo");
  {
    const std::string class_name = "VolumeJunction1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<UserObjectName>>("numerical_flux_names") = _numerical_flux_names;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, volume_junction_uo_name, params);
  }

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
  {
    const std::vector<NonlinearVariableName> var_names = {
        FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "VolumeJunction1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("volume_junction_uo") = volume_junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
      params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
      params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
      params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
      params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
      params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();
      _sim.addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
  }

  // Add scalar kernels for the junction
  std::vector<NonlinearVariableName> var_names(N_EQ);
  var_names[RHOV_INDEX] = _rhoV_var_name;
  var_names[RHOUV_INDEX] = _rhouV_var_name;
  var_names[RHOVV_INDEX] = _rhovV_var_name;
  var_names[RHOWV_INDEX] = _rhowV_var_name;
  var_names[RHOEV_INDEX] = _rhoEV_var_name;
  for (std::size_t i = 0; i < N_EQ; i++)
  {
    {
      const std::string class_name = "ODETimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      _sim.addScalarKernel(class_name, genName(name(), var_names[i], "td"), params);
    }
    {
      const std::string class_name = "VolumeJunctionAdvectionScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      params.set<UserObjectName>("volume_junction_uo") = volume_junction_uo_name;
      params.set<unsigned int>("equation_index") = i;
      _sim.addScalarKernel(class_name, genName(name(), var_names[i], "vja_sk"), params);
    }
  }

  {
    const std::string class_name = "VolumeJunction1PhasePressureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _pressure_var_name;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxScalarKernel(class_name, genName(name(), "pressure_aux"), params);
  }
  {
    const std::string class_name = "VolumeJunction1PhaseTemperatureAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _temperature_var_name;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    params.set<UserObjectName>("fp") = _fp_name;
    _sim.addAuxScalarKernel(class_name, genName(name(), "temperature_aux"), params);
  }
  {
    const std::string class_name = "VolumeJunction1PhaseVelocityMagnitudeAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _velocity_var_name;
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    _sim.addAuxScalarKernel(class_name, genName(name(), "velmag_aux"), params);
  }
}
