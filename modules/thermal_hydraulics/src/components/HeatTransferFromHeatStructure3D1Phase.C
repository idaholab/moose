//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferFromHeatStructure3D1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructureFromFile3D.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"
#include "MooseMesh.h"
#include "ClosuresBase.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", HeatTransferFromHeatStructure3D1Phase);

InputParameters
HeatTransferFromHeatStructure3D1Phase::validParams()
{
  InputParameters params = HeatTransferFromTemperature1Phase::validParams();

  params.makeParamRequired<FunctionName>("P_hf");
  params.makeParamNotRequired<std::string>("flow_channel");
  params.suppressParameter<std::string>("flow_channel");
  params.set<std::string>("flow_channel", true) = "";
  params.addRequiredParam<std::vector<std::string>>(
      "flow_channels", "List of flow channel component names to connect to");
  params.addRequiredParam<BoundaryName>(
      "boundary", "The name of the heat structure boundary this heat transfer is applied on.");
  params.addRequiredParam<std::string>("hs", "Heat structure name");
  params.addClassDescription("Connects multiple 1-phase flow channels and a 3D heat structure");

  return params;
}

HeatTransferFromHeatStructure3D1Phase::HeatTransferFromHeatStructure3D1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters),
    _flow_channel_names(getParam<std::vector<std::string>>("flow_channels")),
    _boundary(getParam<BoundaryName>("boundary")),
    _hs_name(getParam<std::string>("hs")),
    _mesh_alignment(constMesh()),
    _layered_average_uo_direction(MooseEnum("x y z"))
{
  for (const auto & fch_name : _flow_channel_names)
    addDependency(fch_name);
  addDependency(_hs_name);
}

const FEType &
HeatTransferFromHeatStructure3D1Phase::getFEType()
{
  return HeatConductionModel::feType();
}

void
HeatTransferFromHeatStructure3D1Phase::setupMesh()
{
  if (hasComponentByName<HeatStructureFromFile3D>(_hs_name))
  {
    std::vector<dof_id_type> fchs_elem_ids;
    for (unsigned int i = 0; i < _flow_channel_names.size(); i++)
    {
      auto fch_name = _flow_channel_names[i];
      if (hasComponentByName<FlowChannel1Phase>(fch_name))
      {
        const FlowChannel1Phase & flow_channel = getComponentByName<FlowChannel1Phase>(fch_name);
        fchs_elem_ids.insert(fchs_elem_ids.end(),
                             flow_channel.getElementIDs().begin(),
                             flow_channel.getElementIDs().end());
      }
    }
    // Boundary info (element ID, local side number) for the heat structure side
    std::vector<std::tuple<dof_id_type, unsigned short int>> bnd_info;
    BoundaryID bd_id = mesh().getBoundaryID(_boundary);
    mesh().buildBndElemList();
    const auto & bnd_to_elem_map = mesh().getBoundariesToActiveSemiLocalElemIds();
    auto search = bnd_to_elem_map.find(bd_id);
    if (search == bnd_to_elem_map.end())
      mooseDoOnce(logError("The boundary '", _boundary, "' (", bd_id, ") was not found."));
    else
    {
      const std::unordered_set<dof_id_type> & bnd_elems = search->second;
      for (auto elem_id : bnd_elems)
      {
        const Elem * elem = mesh().elemPtr(elem_id);
        unsigned int side = mesh().sideWithBoundaryID(elem, bd_id);
        bnd_info.push_back(std::tuple<dof_id_type, unsigned short int>(elem_id, side));
      }

      _mesh_alignment.initialize(fchs_elem_ids, bnd_info);

      for (const auto & fc_elem_id : fchs_elem_ids)
      {
        if (_mesh_alignment.hasCoupledSecondaryElemIDs(fc_elem_id))
        {
          const auto & hs_elem_ids = _mesh_alignment.getCoupledSecondaryElemIDs(fc_elem_id);
          for (const auto & hs_elem_id : hs_elem_ids)
            getTHMProblem().augmentSparsity(fc_elem_id, hs_elem_id);
        }
      }
    }
  }
}

HeatTransferFromHeatStructure3D1Phase::EAxisAlignment
HeatTransferFromHeatStructure3D1Phase::getFlowChannelAxisAlignment(
    const std::string & flow_channel_name) const
{
  const FlowChannel1Phase & flow_channel = getComponentByName<FlowChannel1Phase>(flow_channel_name);
  RealVectorValue direction = flow_channel.getDirection().unit();
  Real x_dir_norm = std::abs(direction * RealVectorValue(1, 0, 0));
  Real y_dir_norm = std::abs(direction * RealVectorValue(0, 1, 0));
  Real z_dir_norm = std::abs(direction * RealVectorValue(0, 0, 1));
  if (x_dir_norm == 1 && y_dir_norm == 0 && z_dir_norm == 0)
    return EAxisAlignment::X;
  else if (x_dir_norm == 0 && y_dir_norm == 1 && z_dir_norm == 0)
    return EAxisAlignment::Y;
  else if (x_dir_norm == 0 && y_dir_norm == 0 && z_dir_norm == 1)
    return EAxisAlignment::Z;
  else
  {
    logError(
        "The flow channel '", flow_channel_name, "' must be aligned with the x-, y-, or z- axis.");
    return EAxisAlignment::INVALID;
  }
}

void
HeatTransferFromHeatStructure3D1Phase::init()
{
  ConnectorBase::init();

  std::vector<EAxisAlignment> fch_axis_alignment;
  std::vector<unsigned int> fch_num_elems;
  std::vector<Real> fch_lengths;
  for (const auto & fch_name : _flow_channel_names)
  {
    checkComponentOfTypeExistsByName<FlowChannel1Phase>(fch_name);

    if (hasComponentByName<FlowChannel1Phase>(fch_name))
    {
      const FlowChannel1Phase & flow_channel = getComponentByName<FlowChannel1Phase>(fch_name);

      flow_channel.addHeatTransferName(name());
      fch_axis_alignment.push_back(getFlowChannelAxisAlignment(fch_name));

      const auto subdomain_names = flow_channel.getSubdomainNames();
      _flow_channel_subdomains.insert(
          _flow_channel_subdomains.end(), subdomain_names.begin(), subdomain_names.end());
      _flow_channel_closures.push_back(flow_channel.getClosures());

      fch_num_elems.push_back(flow_channel.getNumElems());

      fch_lengths.push_back(flow_channel.getLength());
    }
  }

  // check that all flow channels have the same axis alignment
  if (fch_axis_alignment.size() > 1)
  {
    for (unsigned int i = 1; i < fch_axis_alignment.size(); i++)
      if (fch_axis_alignment[i] != fch_axis_alignment[0])
        logError("Flow channel '",
                 _flow_channel_names[i],
                 "' has a different axis alignment (",
                 fch_axis_alignment[i],
                 "). Make sure all flow channels are aligned with the same axis.");
  }
  if (fch_axis_alignment.size() > 0 && fch_axis_alignment[0] != EAxisAlignment::INVALID)
    _layered_average_uo_direction = fch_axis_alignment[0];

  // check that all flow channels have the same number of elements
  if (fch_num_elems.size() > 1)
  {
    for (unsigned int i = 1; i < fch_num_elems.size(); i++)
      if (fch_num_elems[i] != fch_num_elems[0])
        logError("Flow channel '",
                 _flow_channel_names[i],
                 "' has ",
                 fch_num_elems[i],
                 " elements which is inconsistent with the rest of the flow channels. Make sure "
                 "all flow channels have the same number of elements.");
  }
  if (fch_num_elems.size() > 0)
    _num_layers = fch_num_elems[0];

  // check that all flow channels have the same length
  if (fch_lengths.size() > 1)
  {
    for (unsigned int i = 1; i < fch_lengths.size(); i++)
      if (fch_lengths[i] != fch_lengths[0])
        logError("Flow channel '",
                 _flow_channel_names[i],
                 "' has length equal to ",
                 fch_lengths[i],
                 " which is inconsistent with the rest of the flow channels. Make sure all flow "
                 "channels have the length.");
  }
}

void
HeatTransferFromHeatStructure3D1Phase::initSecondary()
{
  ConnectorBase::initSecondary();

  for (const auto & fch_name : _flow_channel_names)
  {
    if (hasComponentByName<FlowChannel1Phase>(fch_name))
    {
      const FlowChannel1Phase & flow_channel = getComponentByName<FlowChannel1Phase>(fch_name);
      const std::string suffix = flow_channel.getHeatTransferNamesSuffix(name());
      _P_hf_name = FlowModel::HEAT_FLUX_PERIMETER + suffix;
      _T_wall_name = FlowModel::TEMPERATURE_WALL + suffix;
      _q_wall_name = FlowModel::HEAT_FLUX_WALL + suffix;
      _Hw_1phase_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + suffix;
    }
  }
}

void
HeatTransferFromHeatStructure3D1Phase::check() const
{
  ConnectorBase::check();

  for (unsigned int i = 0; i < _flow_channel_closures.size(); i++)
  {
    auto & clsr = _flow_channel_closures[i];
    if (clsr != nullptr && hasComponentByName<FlowChannel1Phase>(_flow_channel_names[i]))
      clsr->checkHeatTransfer(*this, getComponentByName<FlowChannel1Phase>(_flow_channel_names[i]));
  }

  if (!hasComponentByName<HeatStructureFromFile3D>(_hs_name))
    logError("The component '", _hs_name, "' is not a HeatStructureFromFile3D component.");
}

void
HeatTransferFromHeatStructure3D1Phase::addVariables()
{
  getTHMProblem().addSimVariable(
      false, _P_hf_name, getTHMProblem().getFlowFEType(), _flow_channel_subdomains);

  _P_hf_fn_name = getParam<FunctionName>("P_hf");
  getTHMProblem().addFunctionIC(_P_hf_name, _P_hf_fn_name, _flow_channel_subdomains);

  getTHMProblem().addSimVariable(
      false, FlowModel::TEMPERATURE_WALL, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);
  getTHMProblem().addSimVariable(
      false, _T_wall_name, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);

  // wall temperature initial condition
  if (!getTHMProblem().hasInitialConditionsFromFile() && !_app.isRestarting())
  {
    const HeatStructureFromFile3D & hs = getComponentByName<HeatStructureFromFile3D>(_hs_name);
    getTHMProblem().addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_subdomains);
  }
}

void
HeatTransferFromHeatStructure3D1Phase::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  for (unsigned int i = 0; i < _flow_channel_closures.size(); i++)
  {
    _flow_channel_closures[i]->addMooseObjectsHeatTransfer(
        *this, getComponentByName<FlowChannel1Phase>(_flow_channel_names[i]));
  }

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  std::vector<Point> fch_positions;
  for (const auto & fch_name : _flow_channel_names)
  {
    const FlowChannel1Phase & flow_channel = getComponentByName<FlowChannel1Phase>(fch_name);
    fch_positions.push_back(flow_channel.getPosition());
  }

  const UserObjectName T_wall_avg_uo_name = genName(name(), "T_wall_avg_uo");
  {
    const std::string class_name = "NearestPointLayeredSideAverage";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") = {HeatConductionModel::TEMPERATURE};
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    params.set<MooseEnum>("direction") = _layered_average_uo_direction;
    params.set<unsigned int>("num_layers") = _num_layers;
    params.set<std::vector<Point>>("points") = fch_positions;
    getTHMProblem().addUserObject(class_name, T_wall_avg_uo_name, params);
  }
  {
    std::string class_name = "SpatialUserObjectAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    params.set<UserObjectName>("user_object") = T_wall_avg_uo_name;
    getTHMProblem().addAuxKernel(class_name, genName(name(), "T_wall_transfer"), params);
  }
  {
    const std::string class_name = "ADOneD3EqnEnergyHeatFluxFromHeatStructure3D";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<UserObjectName>("user_object") = T_wall_avg_uo_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    getTHMProblem().addKernel(class_name, genName(name(), "heat_flux_kernel"), params);
  }

  const UserObjectName heat_transfer_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "ADHeatTransferFromHeatStructure3D1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MeshAlignment1D3D *>("_mesh_alignment") = &_mesh_alignment;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, heat_transfer_uo_name, params);
  }
  {
    const std::string class_name = "ADConvectionHeatTransfer3DBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<UserObjectName>("ht_uo") = heat_transfer_uo_name;
    getTHMProblem().addBoundaryCondition(class_name, genName(name(), "heat_flux_bc"), params);
  }
}
