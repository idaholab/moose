//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1Phase.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1Phase);

const unsigned int VolumeJunction1Phase::N_EQ = 5;

InputParameters
VolumeJunction1Phase::validParams()
{
  InputParameters params = FlowJunction1Phase::validParams();

  params.addDeprecatedParam<bool>(
      "use_scalar_variables",
      "True if the junction variables are scalar variables",
      "Please remove this parameter; it no longer has any effect. The behavior corresponding to "
      "'use_scalar_variables = false' is now the only option.");

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");

  params.addParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addParam<FunctionName>("initial_vel_x", "Initial velocity in x-direction [m/s]");
  params.addParam<FunctionName>("initial_vel_y", "Initial velocity in y-direction [m/s]");
  params.addParam<FunctionName>("initial_vel_z", "Initial velocity in z-direction [m/s]");

  params.addParam<Real>("scaling_factor_rhoV", 1.0, "Scaling factor for rho*V [-]");
  params.addParam<Real>("scaling_factor_rhouV", 1.0, "Scaling factor for rho*u*V [-]");
  params.addParam<Real>("scaling_factor_rhovV", 1.0, "Scaling factor for rho*v*V [-]");
  params.addParam<Real>("scaling_factor_rhowV", 1.0, "Scaling factor for rho*w*V [-]");
  params.addParam<Real>("scaling_factor_rhoEV", 1.0, "Scaling factor for rho*E*V [-]");

  params.addParam<Real>("K", 0., "Form loss factor [-]");
  params.addParam<Real>("A_ref", "Reference area [m^2]");

  params.addParam<bool>("apply_velocity_scaling",
                        false,
                        "Set to true to apply the scaling to the normal velocity. See "
                        "documentation for more information.");

  params.declareControllable("K");
  params.addClassDescription("Junction between 1-phase flow channels that has a non-zero volume");

  return params;
}

VolumeJunction1Phase::VolumeJunction1Phase(const InputParameters & params)
  : FlowJunction1Phase(params),

    _volume(getParam<Real>("volume")),
    _position(getParam<Point>("position")),

    _scaling_factor_rhoV(getParam<Real>("scaling_factor_rhoV")),
    _scaling_factor_rhouV(getParam<Real>("scaling_factor_rhouV")),
    _scaling_factor_rhovV(getParam<Real>("scaling_factor_rhovV")),
    _scaling_factor_rhowV(getParam<Real>("scaling_factor_rhowV")),
    _scaling_factor_rhoEV(getParam<Real>("scaling_factor_rhoEV")),

    _rhoV_var_name(junctionVariableName("rhoV")),
    _rhouV_var_name(junctionVariableName("rhouV")),
    _rhovV_var_name(junctionVariableName("rhovV")),
    _rhowV_var_name(junctionVariableName("rhowV")),
    _rhoEV_var_name(junctionVariableName("rhoEV")),
    _pressure_var_name(junctionVariableName("p")),
    _temperature_var_name(junctionVariableName("T")),
    _velocity_var_name(junctionVariableName("vel")),

    _K(getParam<Real>("K")),
    _A_ref(isParamValid("A_ref") ? getParam<Real>("A_ref") : _zero)
{
  // Note: 'A_ref' can be required by child classes
  if (!params.isParamRequired("A_ref") && params.isParamSetByUser("A_ref") &&
      !params.isParamSetByUser("K"))
    logWarning("Parameter 'A_ref' is specified, but 'K' is not specified, so the junction will "
               "behave as if there were no form loss.");
}

void
VolumeJunction1Phase::setupMesh()
{
  FlowJunction1Phase::setupMesh();

  // Add a NodeElem to the mesh
  auto * node = addNode(_position);
  auto * elem = addNodeElement(node->id());
  _junction_subdomain_id = mesh().getNextSubdomainId();
  elem->subdomain_id() = _junction_subdomain_id;
  setSubdomainInfo(_junction_subdomain_id, name());

  // Add coupling between the flow channel end elements and the NodeElem
  const auto & elem_ids = getConnectedElementIDs();
  for (unsigned int i = 0; i < elem_ids.size(); i++)
    getTHMProblem().augmentSparsity(elem_ids[i], elem->id());
}

void
VolumeJunction1Phase::check() const
{
  FlowJunction1Phase::check();

  bool ics_set =
      getTHMProblem().hasInitialConditionsFromFile() ||
      (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel_x") &&
       isParamValid("initial_vel_y") && isParamValid("initial_vel_z"));

  if (!ics_set && !_app.isRestarting())
  {
    // create a list of the missing IC parameters
    const std::vector<std::string> ic_params{
        "initial_p", "initial_T", "initial_vel_x", "initial_vel_y", "initial_vel_z"};
    std::ostringstream oss;
    for (const auto & ic_param : ic_params)
      if (!isParamValid(ic_param))
        oss << " " << ic_param;

    logError("The following initial condition parameters are missing:", oss.str());
  }

  // https://github.com/idaholab/moose/issues/28670
  if (getTHMProblem().hasInitialConditionsFromFile() && libMesh::n_threads() > 1 &&
      _app.n_processors() > 1)
    mooseDocumentedError("moose",
                         28670,
                         "Using initial conditions from a file for VolumeJunction1Phase is "
                         "currently not tested for parallel threading.");
}

void
VolumeJunction1Phase::addVariables()
{
  addJunctionVariable(true, _rhoV_var_name, _scaling_factor_rhoV);
  addJunctionVariable(true, _rhouV_var_name, _scaling_factor_rhouV);
  addJunctionVariable(true, _rhovV_var_name, _scaling_factor_rhovV);
  addJunctionVariable(true, _rhowV_var_name, _scaling_factor_rhowV);
  addJunctionVariable(true, _rhoEV_var_name, _scaling_factor_rhoEV);

  addJunctionVariable(false, _pressure_var_name);
  addJunctionVariable(false, _temperature_var_name);
  addJunctionVariable(false, _velocity_var_name);

  if (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel_x") &&
      isParamValid("initial_vel_y") && isParamValid("initial_vel_z"))
  {
    addVolumeJunctionIC(_rhoV_var_name, "rhoV");
    addVolumeJunctionIC(_rhouV_var_name, "rhouV");
    addVolumeJunctionIC(_rhovV_var_name, "rhovV");
    addVolumeJunctionIC(_rhowV_var_name, "rhowV");
    addVolumeJunctionIC(_rhoEV_var_name, "rhoEV");

    addVolumeJunctionIC(_pressure_var_name, "p");
    addVolumeJunctionIC(_temperature_var_name, "T");
    addVolumeJunctionIC(_velocity_var_name, "vel");
  }
}

void
VolumeJunction1Phase::buildVolumeJunctionUserObject()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADVolumeJunction1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<bool>("use_scalar_variables") = false;
    params.set<subdomain_id_type>("junction_subdomain_id") = _junction_subdomain_id;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = getConnectedProcessorIDs();
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
    params.set<Real>("K") = _K;
    params.set<Real>("A_ref") = _A_ref;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("apply_velocity_scaling") = getParam<bool>("apply_velocity_scaling");
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);
    connectObject(params, _junction_uo_name, "K");
  }
}

void
VolumeJunction1Phase::addMooseObjects()
{
  buildVolumeJunctionUserObject();

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
  {
    const std::vector<NonlinearVariableName> var_names = {
        FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "ADVolumeJunction1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();
      getTHMProblem().addBoundaryCondition(
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
      const std::string class_name = "ADTimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      const std::string obj_name = genName(name(), var_names[i], "td");
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      getTHMProblem().addKernel(class_name, obj_name, params);
    }
    {
      const std::string class_name = "ADVolumeJunctionAdvectionKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("equation_index") = i;
      const std::string obj_name = genName(name(), var_names[i], "vja_sk");
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      getTHMProblem().addKernel(class_name, obj_name, params);
    }
  }

  const std::vector<std::pair<std::string, VariableName>> quantities = {
      {"pressure", _pressure_var_name},
      {"temperature", _temperature_var_name},
      {"speed", _velocity_var_name}};
  for (const auto & quantity_and_name : quantities)
  {
    const std::string class_name = "VolumeJunction1PhaseAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = quantity_and_name.second;
    params.set<MooseEnum>("quantity") = quantity_and_name.first;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    params.set<UserObjectName>("fp") = _fp_name;
    const std::string obj_name = genName(name(), quantity_and_name.first + "_aux");
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    getTHMProblem().addAuxKernel(class_name, obj_name, params);
  }

  // An error message results if there is any block without a material, so
  // until this restriction is removed, we must add a dummy material that
  // computes no material properties.
  {
    const std::string class_name = "GenericConstantMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {};
    params.set<std::vector<Real>>("prop_values") = {};
    getTHMProblem().addMaterial(class_name, genName(name(), "dummy_mat"), params);
  }
}

std::string
VolumeJunction1Phase::junctionVariableName(const std::string & var_base) const
{
  return var_base;
}

void
VolumeJunction1Phase::addJunctionVariable(bool is_nonlinear,
                                          const VariableName & var,
                                          Real scaling_factor)
{
  auto & problem = getTHMProblem();

  const libMesh::FEType fe_type(CONSTANT, MONOMIAL);
  const auto & subdomains = getSubdomainNames();

  if (is_nonlinear)
    problem.addSimVariable(is_nonlinear, var, fe_type, subdomains, scaling_factor);
  else
    problem.addSimVariable(is_nonlinear, var, fe_type, subdomains);
}

void
VolumeJunction1Phase::addJunctionIC(const VariableName & var, Real value)
{
  getTHMProblem().addConstantIC(var, value, getSubdomainNames());
}

void
VolumeJunction1Phase::addVolumeJunctionIC(const VariableName & var, const std::string & quantity)
{
  const std::string class_name = "VolumeJunction1PhaseIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<VariableName>("variable") = var;
  params.set<MooseEnum>("quantity") = quantity;
  params.applySpecificParameters(parameters(),
                                 {"initial_p",
                                  "initial_T",
                                  "initial_vel_x",
                                  "initial_vel_y",
                                  "initial_vel_z",
                                  "volume",
                                  "position"});
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  getTHMProblem().addSimInitialCondition(class_name, genName(name(), var, "ic"), params);
}
