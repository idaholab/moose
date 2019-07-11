#include "FlowChannelBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"
#include "InputParameterLogic.h"
#include "StabilizationSettings.h"
#include "HeatTransferBase.h"
#include "HeatTransfer1PhaseBase.h"
#include "HeatTransfer2PhaseBase.h"
#include "ClosuresBase.h"
#include "THMApp.h"
#include "THMMesh.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"

const std::map<std::string, FlowChannelBase::EConvHeatTransGeom>
    FlowChannelBase::_heat_transfer_geom_to_enum{{"PIPE", PIPE}, {"ROD_BUNDLE", ROD_BUNDLE}};

MooseEnum
FlowChannelBase::getConvHeatTransGeometry(const std::string & name)
{
  return THM::getMooseEnum<EConvHeatTransGeom>(name, _heat_transfer_geom_to_enum);
}

template <>
FlowChannelBase::EConvHeatTransGeom
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowChannelBase::EConvHeatTransGeom>(
      s, FlowChannelBase::_heat_transfer_geom_to_enum);
}

template <>
InputParameters
validParams<FlowChannelBase>()
{
  InputParameters params = validParams<GeometricalFlowComponent>();
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addParam<Real>("roughness", 0.0, "roughness, [m]");
  params.addParam<FunctionName>("f", "Wall friction");
  params.addParam<MooseEnum>("heat_transfer_geom",
                             FlowChannelBase::getConvHeatTransGeometry("PIPE"),
                             "Convective heat transfer geometry");
  params.addParam<Real>("PoD", 1, "pitch to diameter ratio for parallel bundle heat transfer");
  params.addParam<bool>(
      "pipe_pars_transferred",
      false,
      "Set to true if Dh, P_hf and A are going to be transferred in from an external source");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter");
  params.addParam<UserObjectName>(
      "stabilization", "", "The name of the local stabilization scheme to use");
  params.addParam<bool>("shock_capturing", false, "Use shock capturing or not (locally)");
  params.addParam<bool>("lump_mass_matrix", false, "Lump the mass matrix");
  params.addRequiredParam<std::string>("closures", "Closures type");

  params.addPrivateParam<std::string>("component_type", "pipe");
  return params;
}

FlowChannelBase::FlowChannelBase(const InputParameters & params)
  : GeometricalFlowComponent(params),
    _flow_model(nullptr),
    _closures_name(getParam<std::string>("closures")),
    _pipe_pars_transferred(getParam<bool>("pipe_pars_transferred")),
    _has_Dh(isParamValid("D_h")),
    _Dh_function(_has_Dh ? getParam<FunctionName>("D_h") : ""),
    _roughness(getParam<Real>("roughness")),
    _HT_geometry(getEnumParam<EConvHeatTransGeom>("heat_transfer_geom")),
    _PoD(getParam<Real>("PoD")),
    _has_PoD(isParamValid("PoD")),
    _is_horizontal(_gravity_angle_type == HORIZONTAL || _gravity_angle_type == MOSTLY_HORIZONTAL),
    _stabilization_uo_name(getParam<UserObjectName>("stabilization")),
    _temperature_mode(false),
    _n_heat_transfer_connections(0)
{
}

std::shared_ptr<const FlowModel>
FlowChannelBase::getFlowModel() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _flow_model;
}

const FunctionName &
FlowChannelBase::getAreaFunctionName() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _area_function;
}

FunctionName
FlowChannelBase::createAreaFunctionAndGetName()
{
  // Area function has already been created; just need to return its name
  return getParam<FunctionName>("A");
}

void
FlowChannelBase::init()
{
  GeometricalFlowComponent::init();

  _area_function = createAreaFunctionAndGetName();

  _flow_model = buildFlowModel();
  _flow_model->init();

  _closures = buildClosures();

  // initialize the stabilization object
  if (!_stabilization_uo_name.empty())
    if (_sim.hasUserObject(_stabilization_uo_name))
    {
      StabilizationSettings & stabilization = const_cast<StabilizationSettings &>(
          _sim.getUserObjectTempl<StabilizationSettings>(_stabilization_uo_name));
      stabilization.initMooseObjects(*_flow_model);
    }
}

std::shared_ptr<ClosuresBase>
FlowChannelBase::buildClosures()
{
  const std::string class_name = _app.getClosuresClassName(_closures_name, _model_id);
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Simulation *>("_sim") = &_sim;
  return _factory.create<ClosuresBase>(class_name, genName(name(), class_name), params);
}

void
FlowChannelBase::initSecondary()
{
  GeometricalFlowComponent::initSecondary();

  // Determine heat transfer mode based on connected heat transfer components;
  // if at least one heat transfer component of temperature component is
  // connected, then it's temperature mode. Otherwise, it's heat flux mode, even
  // if no heat transfer components at all were provided - in that case, a zero
  // heat flux is added.
  for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
  {
    const HeatTransferBase & heat_transfer =
        getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);
    if (heat_transfer.isTemperatureType())
      _temperature_mode = true;
  }
}

void
FlowChannelBase::check() const
{
  GeometricalFlowComponent::check();

  _closures->check(*this);

  // check that stabilization exists
  if (!_stabilization_uo_name.empty())
    if (!_sim.hasUserObject(_stabilization_uo_name))
      logWarning("Requested stabilization '", _stabilization_uo_name, "' does not exist. Typo?");

  // check types of heat transfer for all sources; must be all of same type
  if (_temperature_mode)
    for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
    {
      const HeatTransferBase & heat_transfer =
          getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);
      if (!heat_transfer.isTemperatureType())
        logError("Heat sources for a flow channel must be all of temperature type or all of heat "
                 "flux type");
    }
}

void
FlowChannelBase::buildMeshNodes()
{
  Point p(0, 0, 0);
  for (unsigned int i = 0; i < _node_locations.size(); i++)
  {
    p(0) = _node_locations[i];
    addNode(p);
  }
}

void
FlowChannelBase::buildMesh()
{
  buildMeshNodes();

  MeshBase & the_mesh = _mesh.getMesh();
  BoundaryInfo & boundary_info = the_mesh.get_boundary_info();

  // create nodeset for all nodes for this component
  _nodeset_id = _mesh.getNextBoundaryId();
  _nodeset_name = name();
  boundary_info.nodeset_name(_nodeset_id) = _nodeset_name;

  // Check that the number of nodes is consistent with the number of nodes in case component
  // developers screw up (typically in buildMeshNodes() call)
  if (usingSecondOrderMesh())
  {
    if (_node_ids.size() != (2 * _n_elem + 1))
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }
  else
  {
    if (_node_ids.size() != _n_elem + 1)
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }

  for (auto & node_id : _node_ids)
  {
    const Node * nd = the_mesh.node_ptr(node_id);
    boundary_info.add_node(nd, _nodeset_id);
  }

  // elems
  _subdomain_id = _mesh.getNextSubdomainId();
  setSubdomainInfo(_subdomain_id, name());
  BoundaryID bc_id_inlet = _mesh.getNextBoundaryId();
  BoundaryID bc_id_outlet = _mesh.getNextBoundaryId();
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    Elem * elem = nullptr;
    if (usingSecondOrderMesh())
      elem = addElementEdge3(_node_ids[2 * i], _node_ids[2 * i + 2], _node_ids[2 * i + 1]);
    else
      elem = addElementEdge2(_node_ids[i], _node_ids[i + 1]);
    elem->subdomain_id() = _subdomain_id;

    // BCs
    if (i == 0)
    {
      Point pt = _position;
      _connections[FlowConnection::IN].push_back(
          Connection(pt, elem->node_ptr(0), bc_id_inlet, -1));
      boundary_info.add_side(elem, 0, bc_id_inlet);
      _mesh.setBoundaryName(bc_id_inlet, genName(name(), "in"));
    }
    if (i == (_n_elem - 1))
    {
      Point pt = _position + _length * _dir;
      _connections[FlowConnection::OUT].push_back(
          Connection(pt, elem->node_ptr(1), bc_id_outlet, 1));
      boundary_info.add_side(elem, 1, bc_id_outlet);
      _mesh.setBoundaryName(bc_id_outlet, genName(name(), "out"));
    }
  }
}

void
FlowChannelBase::addVariables()
{
  // This should be called after initSecondary() because it relies on the names
  // generated in initSecondary() of heat transfer components
  getHeatTransferVariableNames();

  _flow_model->addVariables();

  // wall heat flux
  if (!_temperature_mode && _n_heat_transfer_connections != 1)
    _sim.addVariable(false, FlowModel::HEAT_FLUX_WALL, _sim.getFlowFEType(), _subdomain_id);

  // total heat flux perimeter
  if (_n_heat_transfer_connections > 1)
  {
    const std::string class_name = "SumIC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<VariableName>("variable") = FlowModel::HEAT_FLUX_PERIMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("values") = _P_hf_names;
    _sim.addInitialCondition(class_name, genName(name(), "P_hf_ic"), params);
  }

  _flow_model->addInitialConditions();

  if (!_stabilization_uo_name.empty())
  {
    const StabilizationSettings & stabilization =
        _sim.getUserObjectTempl<StabilizationSettings>(_stabilization_uo_name);
    stabilization.addVariables(*_flow_model, _subdomain_id);
  }
}

void
FlowChannelBase::setupDh()
{
  const std::string nm = genName(name(), "D_h_material");

  if (_has_Dh)
  {
    const std::string class_name = "GenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {FlowModel::HYDRAULIC_DIAMETER};
    params.set<std::vector<FunctionName>>("prop_values") = {_Dh_function};
    _sim.addMaterial(class_name, nm, params);

    makeFunctionControllableIfConstant(_Dh_function, "D_h");
  }
  else
  {
    const std::string class_name = "HydraulicDiameterCircularMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addMaterial(class_name, nm, params);
  }
}

void
FlowChannelBase::addCommonObjects()
{
  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};

  {
    std::string class_name = "DirectionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    _sim.addMaterial(class_name, genName(name(), "dir_mat"), params);
  }

  if (!_pipe_pars_transferred)
  {
    // Area
    if (_spatial_discretization == FlowModel::rDG)
    {
      {
        std::string class_name = "FunctionAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = FlowModel::AREA_LINEAR;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<FunctionName>("function") = _area_function;
        params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
        const std::string aux_kernel_name = genName(name(), "area_linear_aux");
        _sim.addAuxKernel(class_name, aux_kernel_name, params);
        makeFunctionControllableIfConstant(_area_function, "Area");
      }
      {
        const std::string class_name = "CopyValueAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = FlowModel::AREA;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("source") = {FlowModel::AREA_LINEAR};
        params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
        const std::string aux_kernel_name = genName(name(), "area_aux");
        _sim.addAuxKernel(class_name, aux_kernel_name, params);
        makeFunctionControllableIfConstant(_area_function, "Area");
      }
    }
    else
    {
      std::string class_name = "FunctionAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = FlowModel::AREA;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<FunctionName>("function") = _area_function;
      params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
      const std::string aux_kernel_name = genName(name(), "area_linear_aux");
      _sim.addAuxKernel(class_name, aux_kernel_name, params);
      makeFunctionControllableIfConstant(_area_function, "Area");
    }

    setupDh();
  }

  ExecFlagEnum execute_on_initial_linear(MooseUtils::getDefaultExecFlagEnum());
  execute_on_initial_linear = {EXEC_INITIAL, EXEC_LINEAR};

  // total heat flux perimeter aux kernel
  if (_n_heat_transfer_connections > 1)
  {
    const std::string class_name = "SumAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModel::HEAT_FLUX_PERIMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("values") = _P_hf_names;
    params.set<ExecFlagEnum>("execute_on") = execute_on_initial_linear;
    _sim.addAuxKernel(class_name, genName(name(), "P_hf_auxkernel"), params);
  }

  // weighted average wall heat flux aux kernel
  if (!_temperature_mode)
  {
    if (_n_heat_transfer_connections > 1)
    {
      const std::string class_name = "WeightedAverageMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("prop_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<MaterialPropertyName>>("values") = _q_wall_names;
      params.set<std::vector<VariableName>>("weights") = _P_hf_names;
      _sim.addMaterial(class_name, genName(name(), class_name, FlowModel::HEAT_FLUX_WALL), params);
    }
    else if (_n_heat_transfer_connections == 0)
    {
      const std::string class_name = "ConstantMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::string>("property_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<Real>("value") = 0;
      _sim.addMaterial(class_name, genName(name(), class_name, FlowModel::HEAT_FLUX_WALL), params);
    }
  }
}

void
FlowChannelBase::addMooseObjects()
{
  addCommonObjects();

  _flow_model->addMooseObjects();
  _closures->addMooseObjects(*this);

  if (!_stabilization_uo_name.empty())
  {
    InputParameters pars = emptyInputParameters();
    pars.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    pars.set<Component *>("component") = this;
    pars.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
    pars.set<RealVectorValue>("gravity_vector") = _gravity_vector;

    const StabilizationSettings & stabilization =
        _sim.getUserObjectTempl<StabilizationSettings>(_stabilization_uo_name);
    stabilization.addMooseObjects(*_flow_model, pars);
  }
}

void
FlowChannelBase::addHeatTransferName(const std::string & name) const
{
  _heat_transfer_names.push_back(name);
  _n_heat_transfer_connections++;
}

void
FlowChannelBase::getHeatTransferVariableNames()
{
  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const HeatTransferBase & heat_transfer =
        getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);

    _P_hf_names.push_back(heat_transfer.getHeatedPerimeterName());
    _T_wall_names.push_back(heat_transfer.getWallTemperatureName());
    _q_wall_names.push_back(heat_transfer.getWallHeatFluxName());
  }
}

std::string
FlowChannelBase::getHeatTransferNamesSuffix(const std::string & ht_name) const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  // if there is more than one connected heat transfer component, then number them
  if (_n_heat_transfer_connections > 1)
  {
    // determine index of heat transfer name based on when it was added
    auto it = std::find(_heat_transfer_names.begin(), _heat_transfer_names.end(), ht_name);
    if (it != _heat_transfer_names.end())
    {
      const unsigned int index = std::distance(_heat_transfer_names.begin(), it);
      const std::string suffix = ":" + std::to_string(index + 1);
      return suffix;
    }
    else
      mooseError(
          "Heat transfer component '", ht_name, "' was not added to flow channel '", name(), "'");
  }
  // else, don't add a suffix; there is no need
  else
    return "";
}

unsigned int
FlowChannelBase::getNodesetID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_id;
}

const BoundaryName &
FlowChannelBase::getNodesetName() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_name;
}

SubdomainID
FlowChannelBase::getSubdomainID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_id;
}
