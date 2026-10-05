//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseAC.h"
#include "FlowChannel1PhaseAC.h"
#include "THMNames.h"
#include "THMUtils.h"
#include "AugmentSparsityBetweenElements.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "MooseMesh.h"

// Registered under the classic Component's name ('VolumeJunction1Phase') rather than this class's
// own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_mesh_generator");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_variable");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_aux_variable");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_ic");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_bc");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_kernel");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_aux_kernel");
registerMooseActionAliased("ThermalHydraulicsApp",
                           VolumeJunction1PhaseAC,
                           "VolumeJunction1Phase",
                           "add_material");
registerActionComponentAliased("ThermalHydraulicsApp",
                               VolumeJunction1PhaseAC,
                               "VolumeJunction1Phase");

const unsigned int VolumeJunction1PhaseAC::N_EQ = 5;

InputParameters
VolumeJunction1PhaseAC::validParams()
{
  InputParameters params = ActionComponent::validParams();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "connections",
      "The flow channel boundaries to connect, each in the form 'component_name:in' or "
      "'component_name:out'");

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");

  params.addRequiredParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addRequiredParam<FunctionName>("initial_vel_x", "Initial velocity in x-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_y", "Initial velocity in y-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_z", "Initial velocity in z-direction [m/s]");

  params.addParam<Real>("scaling_factor_rhoV", 1.0, "Scaling factor for rho*V [-]");
  params.addParam<Real>("scaling_factor_rhouV", 1.0, "Scaling factor for rho*u*V [-]");
  params.addParam<Real>("scaling_factor_rhovV", 1.0, "Scaling factor for rho*v*V [-]");
  params.addParam<Real>("scaling_factor_rhowV", 1.0, "Scaling factor for rho*w*V [-]");
  params.addParam<Real>("scaling_factor_rhoEV", 1.0, "Scaling factor for rho*E*V [-]");

  params.addParam<Real>("K", 0., "Form loss factor [-]");
  params.addParam<Real>("A_ref", 0., "Reference area [m^2]");
  params.declareControllable("K");

  params.addParam<bool>("apply_velocity_scaling",
                        false,
                        "Set to true to apply the scaling to the normal velocity. See "
                        "documentation for more information.");

  params.addClassDescription(
      "Junction with a non-zero volume connecting an arbitrary number of flow channels for "
      "1-phase flow.");

  return params;
}

VolumeJunction1PhaseAC::VolumeJunction1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    _connections(getParam<std::vector<BoundaryName>>("connections")),
    _volume(getParam<Real>("volume")),
    _position(getParam<Point>("position")),
    _scaling_factor_rhoV(getParam<Real>("scaling_factor_rhoV")),
    _scaling_factor_rhouV(getParam<Real>("scaling_factor_rhouV")),
    _scaling_factor_rhovV(getParam<Real>("scaling_factor_rhovV")),
    _scaling_factor_rhowV(getParam<Real>("scaling_factor_rhowV")),
    _scaling_factor_rhoEV(getParam<Real>("scaling_factor_rhoEV")),
    _K(getParam<Real>("K")),
    _A_ref(getParam<Real>("A_ref")),
    _apply_velocity_scaling(getParam<bool>("apply_velocity_scaling")),
    _junction_uo_name(genName(name(), "junction_uo")),
    _coordinator(THMVariableCoordinator::findOrCreate(_awh, _action_factory)),
    _rm_added(false),
    _rhoV_var_name("rhoV"),
    _rhouV_var_name("rhouV"),
    _rhovV_var_name("rhovV"),
    _rhowV_var_name("rhowV"),
    _rhoEV_var_name("rhoEV"),
    _pressure_var_name("p"),
    _temperature_var_name("T"),
    _velocity_var_name("vel")
{
  if (_connections.size() == 0)
    paramError("connections", "There must be at least one connection.");

  for (const auto & connection : _connections)
  {
    _connected_component_names.push_back(THM::parseConnectedComponentName(connection));
    _normals.push_back(THM::parseConnectionNormal(connection));
  }

  _dimension = 0;
  _blocks = {name()};

  addRequiredTask("add_mesh_generator");
  addRequiredTask("add_variable");
  addRequiredTask("add_aux_variable");
  addRequiredTask("add_ic");
  addRequiredTask("add_user_object");
  addRequiredTask("add_bc");
  addRequiredTask("add_kernel");
  addRequiredTask("add_aux_kernel");
  addRequiredTask("add_material");
}

const FlowChannel1PhasePhysics &
VolumeJunction1PhaseAC::connectedPhysics(unsigned int i) const
{
  return _awh.getAction<FlowChannel1PhaseAC>(_connected_component_names[i]).getPhysics();
}

UserObjectName
VolumeJunction1PhaseAC::fluidPropertiesName() const
{
  const UserObjectName fp_name = connectedPhysics(0).fluidPropertiesName();
  for (unsigned int i = 1; i < _connected_component_names.size(); i++)
    if (connectedPhysics(i).fluidPropertiesName() != fp_name)
      mooseError(name(), ": All connected flow channels must use the same fluid properties.");
  return fp_name;
}

std::vector<UserObjectName>
VolumeJunction1PhaseAC::dependsOnUserObjects() const
{
  std::vector<UserObjectName> deps;
  for (unsigned int i = 0; i < _connected_component_names.size(); i++)
    deps.push_back(connectedPhysics(i).numericalFluxUserObjectName());
  return deps;
}

void
VolumeJunction1PhaseAC::addMeshGenerators()
{
  const std::string class_name = "ElementGenerator";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<Point>>("nodal_positions") = {_position};
  params.set<std::vector<dof_id_type>>("element_connectivity") = {0};
  params.set<MooseEnum>("elem_type") = "NODEELEM";
  params.set<SubdomainName>("subdomain_name") = name();
  _app.getMeshGeneratorSystem().addMeshGenerator(class_name, name() + "_base", params);
  _mg_names.push_back(name() + "_base");
  _top_mg_name = _mg_names.back();
}

void
VolumeJunction1PhaseAC::addSolverVariables()
{
  const std::vector<VariableName> var_names = {
      _rhoV_var_name, _rhouV_var_name, _rhovV_var_name, _rhowV_var_name, _rhoEV_var_name};
  const std::vector<Real> scaling_factors = {_scaling_factor_rhoV,
                                             _scaling_factor_rhouV,
                                             _scaling_factor_rhovV,
                                             _scaling_factor_rhowV,
                                             _scaling_factor_rhoEV};
  for (unsigned int i = 0; i < N_EQ; i++)
    _coordinator.requestVariable(
        true, var_names[i], "MONOMIAL", "CONSTANT", scaling_factors[i], _blocks);
}

void
VolumeJunction1PhaseAC::addAuxiliaryVariables()
{
  for (const auto & var_name : {_pressure_var_name, _temperature_var_name, _velocity_var_name})
    _coordinator.requestVariable(false, var_name, "MONOMIAL", "CONSTANT", 1.0, _blocks);
}

void
VolumeJunction1PhaseAC::addVolumeJunctionIC(const VariableName & var_name,
                                            const std::string & quantity)
{
  const std::string class_name = "VolumeJunction1PhaseIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = _blocks;
  params.set<VariableName>("variable") = var_name;
  params.set<MooseEnum>("quantity") = quantity;
  params.applySpecificParameters(parameters(),
                                 {"initial_p",
                                  "initial_T",
                                  "initial_vel_x",
                                  "initial_vel_y",
                                  "initial_vel_z",
                                  "volume",
                                  "position"});
  params.set<UserObjectName>("fluid_properties") = fluidPropertiesName();
  getProblem().addInitialCondition(class_name, genName(name(), var_name, "ic"), params);
}

void
VolumeJunction1PhaseAC::addInitialConditions()
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

void
VolumeJunction1PhaseAC::addUserObjects()
{
  auto & mesh = getProblem().mesh();
  const subdomain_id_type junction_subdomain_id = mesh.getSubdomainID(name());

  // Build the element-adjacency map for AugmentSparsityBetweenElements now that the mesh exists:
  // the junction's own NodeElem needs coupling to every connected flow channel's end element (but
  // those end elements do not need coupling to each other - see the class documentation).
  {
    dof_id_type junction_elem_id = libMesh::DofObject::invalid_id;
    for (const auto & elem : mesh.getMesh().active_element_ptr_range())
      if (elem->subdomain_id() == junction_subdomain_id)
      {
        junction_elem_id = elem->id();
        break;
      }
    if (junction_elem_id == libMesh::DofObject::invalid_id)
      mooseError(name(), ": could not find the junction's own NodeElem in the mesh.");

    const auto boundary_ids = mesh.getBoundaryIDs(_connections, true);
    for (const auto & bid : boundary_ids)
    {
      dof_id_type found_elem_id = libMesh::DofObject::invalid_id;
      for (const auto & side_tuple : mesh.buildSideList())
        if (std::get<2>(side_tuple) == bid)
        {
          found_elem_id = std::get<0>(side_tuple);
          break;
        }
      if (found_elem_id == libMesh::DofObject::invalid_id)
        mooseError(name(), ": could not find an element on one of the 'connections'.");
      _elem_map[junction_elem_id].push_back(found_elem_id);
      _elem_map[found_elem_id].push_back(junction_elem_id);
    }
  }

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  std::vector<UserObjectName> numerical_flux_names;
  for (unsigned int i = 0; i < _connected_component_names.size(); i++)
    numerical_flux_names.push_back(connectedPhysics(i).numericalFluxUserObjectName());

  const std::string class_name = "ADVolumeJunction1PhaseUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<bool>("use_scalar_variables") = false;
  params.set<subdomain_id_type>("junction_subdomain_id") = junction_subdomain_id;
  params.set<std::vector<BoundaryName>>("boundary") = _connections;
  params.set<std::vector<Real>>("normals") = _normals;
  params.set<std::vector<UserObjectName>>("numerical_flux_names") = numerical_flux_names;
  params.set<Real>("volume") = _volume;
  params.set<std::vector<VariableName>>("A") = {THM::AREA};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
  params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
  params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
  params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
  params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
  params.set<Real>("K") = _K;
  params.set<Real>("A_ref") = _A_ref;
  params.set<UserObjectName>("fp") = fluidPropertiesName();
  params.set<bool>("apply_velocity_scaling") = _apply_velocity_scaling;
  params.set<ExecFlagEnum>("execute_on") = execute_on;
  getProblem().addUserObject(class_name, _junction_uo_name, params);
}

void
VolumeJunction1PhaseAC::addWeakBCs()
{
  for (std::size_t i = 0; i < _connections.size(); i++)
    for (const auto & var : {THM::RHOA, THM::RHOUA, THM::RHOEA})
    {
      const std::string class_name = "ADVolumeJunction1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_connections[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var;
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {THM::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
      params.set<bool>("implicit") = THM::implicitTimeIntegrationFlag(_app);
      getProblem().addBoundaryCondition(
          class_name, genName(name(), i, var + ":volume_junction_bc"), params);
    }
}

void
VolumeJunction1PhaseAC::addKernels()
{
  const std::vector<VariableName> var_names = {
      _rhoV_var_name, _rhouV_var_name, _rhovV_var_name, _rhowV_var_name, _rhoEV_var_name};
  for (unsigned int i = 0; i < N_EQ; i++)
  {
    {
      const std::string class_name = "ADTimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      params.set<std::vector<SubdomainName>>("block") = _blocks;
      getProblem().addKernel(class_name, genName(name(), var_names[i], "td"), params);
    }
    {
      const std::string class_name = "ADVolumeJunctionAdvectionKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("equation_index") = i;
      params.set<std::vector<SubdomainName>>("block") = _blocks;
      getProblem().addKernel(class_name, genName(name(), var_names[i], "vja"), params);
    }
  }
}

void
VolumeJunction1PhaseAC::addAuxiliaryKernels()
{
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
    params.set<UserObjectName>("fp") = fluidPropertiesName();
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    getProblem().addAuxKernel(class_name, genName(name(), quantity_and_name.first, "aux"), params);
  }
}

void
VolumeJunction1PhaseAC::addMaterials()
{
  // An error message results if there is any block without a material, so until this restriction
  // is removed, we must add a dummy material that computes no material properties.
  const std::string class_name = "GenericConstantMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = _blocks;
  params.set<std::vector<std::string>>("prop_names") = {};
  params.set<std::vector<Real>>("prop_values") = {};
  getProblem().addMaterial(class_name, genName(name(), "dummy_mat"), params);
}

void
VolumeJunction1PhaseAC::actOnAdditionalTasks()
{
  if (_current_task == "add_aux_variable")
    addAuxiliaryVariables();
  else if (_current_task == "add_ic")
    addInitialConditions();
  else if (_current_task == "add_bc")
    addWeakBCs();
  else if (_current_task == "add_kernel")
    addKernels();
  else if (_current_task == "add_aux_kernel")
    addAuxiliaryKernels();
}

void
VolumeJunction1PhaseAC::addRelationshipManagers(Moose::RelationshipManagerType when_type)
{
  // This virtual is called once per RelationshipManagerType category (geometric, algebraic,
  // coupling); add the (single) RM only once, on the first call. _elem_map is populated later, in
  // addUserObjects() - the RM holds a reference to it, consulted only once mesh distribution
  // actually needs it, well after the mesh (and thus _elem_map) exist.
  if (_rm_added || when_type != Moose::RelationshipManagerType::GEOMETRIC)
    return;
  _rm_added = true;

  const std::string class_name = "AugmentSparsityBetweenElements";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::COUPLING | Moose::RelationshipManagerType::ALGEBRAIC |
      Moose::RelationshipManagerType::GEOMETRIC;
  params.set<std::string>("for_whom") = name();
  params.set<MooseMesh *>("mesh") = _awh.getMesh().get();
  params.set<std::map<dof_id_type, std::vector<dof_id_type>> *>("_elem_map") = &_elem_map;
  auto rm =
      _factory.create<RelationshipManager>(class_name, genName(name(), "sparsity_rm"), params);
  if (!_app.addRelationshipManager(rm))
    _factory.releaseSharedObjects(*rm);
}
