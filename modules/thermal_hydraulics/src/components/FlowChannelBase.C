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
  InputParameters params = Component1D::validParams();
  params += GravityInterface::validParams();

  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object");
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

  params.setDocString(
      "orientation",
      "Direction of flow channel from start position to end position (no need to normalize). For "
      "curved flow channels, it is the (tangent) direction at the start position.");

  params.addPrivateParam<std::string>("component_type", "pipe");
  params.declareControllable("A f");
  params.addParamNamesToGroup("lump_mass_matrix", "Numerical scheme");

  return params;
}

FlowChannelBase::FlowChannelBase(const InputParameters & params)
  : Component1D(params),
    GravityInterface(params),

    _flow_model(nullptr),
    _fp_name(getParam<UserObjectName>("fp")),
    _gravity_angle(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)
                       ? 0.0
                       : std::acos(_dir * _gravity_vector / (_dir.norm() * _gravity_magnitude)) *
                             180 / M_PI),
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
  Component1D::init();

  _area_function = createAreaFunctionAndGetName();

  _flow_model = buildFlowModel();
  if (_flow_model)
  {
    _flow_model->init();

    if (getTHMProblem().hasClosures(_closures_name))
      _closures = getTHMProblem().getClosures(_closures_name);
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
  params.set<THMProblem *>("_thm_problem") = &getTHMProblem();
  params.set<Logger *>("_logger") = &getTHMProblem().log();
  return _factory.create<ClosuresBase>(
      class_name, genName(name(), "closure", _closures_name), params);
}

void
FlowChannelBase::initSecondary()
{
  Component1D::initSecondary();

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
  Component1D::check();

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
    getTHMProblem().addSimInitialCondition(class_name, genName(name(), "P_hf_ic"), params);
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
    getTHMProblem().addMaterial(class_name, genName(name(), "dir_mat"), params);
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
      getTHMProblem().addAuxKernel(class_name, aux_kernel_name, params);
      makeFunctionControllableIfConstant(_area_function, "Area");
    }
    {
      const std::string class_name = "ProjectionAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = FlowModel::AREA;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<VariableName>>("v") = {FlowModel::AREA_LINEAR};
      params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
      const std::string aux_kernel_name = genName(name(), "area_aux");
      getTHMProblem().addAuxKernel(class_name, aux_kernel_name, params);
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
    getTHMProblem().addAuxKernel(class_name, genName(name(), "P_hf_auxkernel"), params);
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
      getTHMProblem().addMaterial(
          class_name, genName(name(), FlowModel::HEAT_FLUX_WALL, "w_avg_mat"), params);
    }
    else if (_n_heat_transfer_connections == 0)
    {
      const std::string class_name = "ADConstantMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::string>("property_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<Real>("value") = 0;
      getTHMProblem().addMaterial(
          class_name, genName(name(), FlowModel::HEAT_FLUX_WALL, "zero_mat"), params);
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
    _T_wall_mat_names.push_back(heat_transfer.getWallTemperatureMatName());
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
