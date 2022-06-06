//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelBase.h"
#include "HeatTransferBase.h"
#include "ClosuresBase.h"
#include "ThermalHydraulicsApp.h"
#include "THMMesh.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"

const std::map<std::string, FlowChannelBase::EConvHeatTransGeom>
    FlowChannelBase::_heat_transfer_geom_to_enum{{"PIPE", PIPE}, {"ROD_BUNDLE", ROD_BUNDLE}};

const std::map<std::string, FlowChannelBase::EPipeType> FlowChannelBase::_pipe_type_to_enum{
    {"STRAIGHT", STRAIGHT}, {"CURVED", CURVED}, {"DOWNCOMER", DOWNCOMER}};

MooseEnum
FlowChannelBase::getConvHeatTransGeometry(const std::string & name)
{
  return THM::getMooseEnum<EConvHeatTransGeom>(name, _heat_transfer_geom_to_enum);
}

MooseEnum
FlowChannelBase::getPipeType(const std::string & name)
{
  return THM::getMooseEnum<EPipeType>(name, _pipe_type_to_enum);
}

template <>
FlowChannelBase::EConvHeatTransGeom
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowChannelBase::EConvHeatTransGeom>(
      s, FlowChannelBase::_heat_transfer_geom_to_enum);
}

template <>
FlowChannelBase::EPipeType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowChannelBase::EPipeType>(s, FlowChannelBase::_pipe_type_to_enum);
}

InputParameters
FlowChannelBase::validParams()
{
  InputParameters params = GeometricalFlowComponent::validParams();
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addParam<Real>("roughness", 0.0, "Roughness [m]");
  params.addParam<FunctionName>("f", "Wall friction factor [-]");
  params.addParam<MooseEnum>("heat_transfer_geom",
                             FlowChannelBase::getConvHeatTransGeometry("PIPE"),
                             "Convective heat transfer geometry");
  params.addParam<Real>("PoD", 1, "Pitch-to-diameter ratio for parallel bundle heat transfer [-]");
  params.addParam<bool>(
      "pipe_pars_transferred",
      false,
      "Set to true if Dh, P_hf and A are going to be transferred in from an external source");
  params.addParam<bool>("lump_mass_matrix", false, "Lump the mass matrix");
  params.addRequiredParam<std::string>("closures", "Closures type");

  params.addPrivateParam<std::string>("component_type", "pipe");

  params.declareControllable("A f");

  return params;
}

FlowChannelBase::FlowChannelBase(const InputParameters & params)
  : GeometricalFlowComponent(params),
    _flow_model(nullptr),
    _closures_name(getParam<std::string>("closures")),
    _pipe_pars_transferred(getParam<bool>("pipe_pars_transferred")),
    _roughness(getParam<Real>("roughness")),
    _HT_geometry(getEnumParam<EConvHeatTransGeom>("heat_transfer_geom")),
    _PoD(getParam<Real>("PoD")),
    _has_PoD(isParamValid("PoD")),
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
  if (_flow_model)
  {
    _flow_model->init();

    if (_sim.hasClosures(_closures_name))
      _closures = _sim.getClosures(_closures_name);
    else
      _closures = buildClosures();
  }
}

std::shared_ptr<ClosuresBase>
FlowChannelBase::buildClosures()
{
  const std::string class_name =
      ThermalHydraulicsApp::getClosuresClassName(_closures_name, getFlowModelID());
  InputParameters params = _factory.getValidParams(class_name);
  params.set<THMProblem *>("_thm_problem") = &_sim;
  params.set<Logger *>("_logger") = &_sim.log();
  return _factory.create<ClosuresBase>(
      class_name, genName(name(), "closure", _closures_name), params);
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

  if (_closures)
    _closures->checkFlowChannel(*this);

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
  BoundaryID bc_id_inlet = _mesh.getNextBoundaryId();
  BoundaryID bc_id_outlet = _mesh.getNextBoundaryId();
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    Elem * elem = nullptr;
    if (usingSecondOrderMesh())
      elem = addElementEdge3(_node_ids[2 * i], _node_ids[2 * i + 2], _node_ids[2 * i + 1]);
    else
      elem = addElementEdge2(_node_ids[i], _node_ids[i + 1]);

    // BCs
    if (i == 0)
    {
      Point pt = _position;
      _connections[Component1DConnection::IN].push_back(
          Connection(pt, elem->node_ptr(0), bc_id_inlet, -1));
      boundary_info.add_side(elem, 0, bc_id_inlet);
      _mesh.setBoundaryName(bc_id_inlet, genName(name(), "in"));
    }
    if (i == (_n_elem - 1))
    {
      Point pt = _position + _length * _dir;
      _connections[Component1DConnection::OUT].push_back(
          Connection(pt, elem->node_ptr(1), bc_id_outlet, 1));
      boundary_info.add_side(elem, 1, bc_id_outlet);
      _mesh.setBoundaryName(bc_id_outlet, genName(name(), "out"));
    }
  }

  if (_axial_region_names.size() > 0)
  {
    unsigned int k = 0;
    for (unsigned int i = 0; i < _axial_region_names.size(); i++)
    {
      const std::string & region_name = _axial_region_names[i];
      SubdomainID subdomain_id = _mesh.getNextSubdomainId();
      setSubdomainInfo(subdomain_id, genName(name(), region_name));

      for (unsigned int j = 0; j < _n_elems[i]; j++, k++)
      {
        dof_id_type elem_id = _elem_ids[k];
        _mesh.elemPtr(elem_id)->subdomain_id() = subdomain_id;
      }
    }
  }
  else
  {
    SubdomainID subdomain_id = _mesh.getNextSubdomainId();
    setSubdomainInfo(subdomain_id, name());

    for (auto && id : _elem_ids)
      _mesh.elemPtr(id)->subdomain_id() = subdomain_id;
  }
}

void
FlowChannelBase::addVariables()
{
  // This should be called after initSecondary() because it relies on the names
  // generated in initSecondary() of heat transfer components
  getHeatTransferVariableNames();

  _flow_model->addVariables();

  // total heat flux perimeter
  if (_n_heat_transfer_connections > 1)
  {
    const std::string class_name = "SumIC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<VariableName>("variable") = FlowModel::HEAT_FLUX_PERIMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("values") = _P_hf_names;
    _sim.addSimInitialCondition(class_name, genName(name(), "P_hf_ic"), params);
  }

  _flow_model->addInitialConditions();
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
}

void
FlowChannelBase::addMooseObjects()
{
  addCommonObjects();

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
      const std::string class_name = "ADWeightedAverageMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("prop_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<MaterialPropertyName>>("values") = _q_wall_names;
      params.set<std::vector<VariableName>>("weights") = _P_hf_names;
      _sim.addMaterial(class_name, genName(name(), FlowModel::HEAT_FLUX_WALL, "w_avg_mat"), params);
    }
    else if (_n_heat_transfer_connections == 0)
    {
      const std::string class_name = "ADConstantMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::string>("property_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<Real>("value") = 0;
      _sim.addMaterial(class_name, genName(name(), FlowModel::HEAT_FLUX_WALL, "zero_mat"), params);
    }
  }

  _flow_model->addMooseObjects();
  _closures->addMooseObjectsFlowChannel(*this);
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

std::vector<std::string>
FlowChannelBase::getHeatTransferNames() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _heat_transfer_names;
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
