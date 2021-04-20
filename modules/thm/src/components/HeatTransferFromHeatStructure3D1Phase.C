#include "HeatTransferFromHeatStructure3D1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructureBase.h"
#include "HeatStructureFromFile3D.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"
#include "MooseMesh.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure3D1Phase);

InputParameters
HeatTransferFromHeatStructure3D1Phase::validParams()
{
  InputParameters params = HeatTransferFromTemperature1Phase::validParams();

  params.addRequiredParam<BoundaryName>("boundary",
                                        "List of boundary names for which this component applies");
  params.addRequiredParam<std::string>("hs", "Heat structure name");
  params.addClassDescription("Connects a 1-phase flow channel and a 3D heat structure");

  return params;
}

HeatTransferFromHeatStructure3D1Phase::HeatTransferFromHeatStructure3D1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters),
    _boundary(getParam<BoundaryName>("boundary")),
    _hs_name(getParam<std::string>("hs")),
    _fch_alignment(_mesh),
    _layered_average_uo_direction(MooseEnum("x y z"))
{
}

const FEType &
HeatTransferFromHeatStructure3D1Phase::getFEType()
{
  return HeatConductionModel::feType();
}

void
HeatTransferFromHeatStructure3D1Phase::setupMesh()
{
  if (hasComponentByName<HeatStructureBase>(_hs_name) &&
      hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);

    /// Boundary info for the heat structure side
    std::vector<std::tuple<dof_id_type, unsigned short int>> bnd_info;
    BoundaryID bd_id = _mesh.getBoundaryID(_boundary);
    _mesh.buildBndElemList();
    const auto & bnd_to_elem_map = _mesh.getBoundariesToElems();
    auto search = bnd_to_elem_map.find(bd_id);
    if (search == bnd_to_elem_map.end())
      mooseError("Error computing heat transfer from 3D heat structure; the boundary id ",
                 bd_id,
                 " is invalid");
    const std::unordered_set<dof_id_type> & bnd_elems = search->second;

    for (auto elem_id : bnd_elems)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      unsigned int side = _mesh.sideWithBoundaryID(elem, bd_id);
      bnd_info.push_back(std::tuple<dof_id_type, unsigned short int>(elem_id, side));
    }

    _fch_alignment.build(bnd_info, flow_channel.getElementIDs());

    for (auto & elem_id : flow_channel.getElementIDs())
    {
      dof_id_type nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);
      if (nearest_elem_id != DofObject::invalid_id)
        _sim.augmentSparsity(elem_id, nearest_elem_id);
    }
  }
}

void
HeatTransferFromHeatStructure3D1Phase::init()
{

  HeatTransferFromTemperature1Phase::init();
  if (hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);
    RealVectorValue direction = flow_channel.getDirection().unit();
    Real x_dir_norm = direction * RealVectorValue(1, 0, 0);
    Real y_dir_norm = direction * RealVectorValue(0, 1, 0);
    Real z_dir_norm = direction * RealVectorValue(0, 0, 1);
    if (x_dir_norm == 1 && y_dir_norm == 0 && z_dir_norm == 0)
      _layered_average_uo_direction = "x";
    else if (x_dir_norm == 0 && y_dir_norm == 1 && z_dir_norm == 0)
      _layered_average_uo_direction = "y";
    else if (x_dir_norm == 0 && y_dir_norm == 0 && z_dir_norm == 1)
      _layered_average_uo_direction = "z";
    else
      logError("The flow channel component must be aligned with the x-, y-, or z- axis.");
  }
}

void
HeatTransferFromHeatStructure3D1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();

  if (!hasComponentByName<HeatStructureFromFile3D>(_hs_name))
    logError("The component '", _hs_name, "' is not a HeatStructureFromFile3D component.");
}

void
HeatTransferFromHeatStructure3D1Phase::addVariables()
{

  HeatTransfer1PhaseBase::addVariables();

  _sim.addSimVariable(
      false, FlowModel::TEMPERATURE_WALL, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);
  _sim.addSimVariable(false, _T_wall_name, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);

  // wall temperature initial condition
  if (!_app.isRestarting())
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    _sim.addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_subdomains);
  }
}

void
HeatTransferFromHeatStructure3D1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  const UserObjectName T_wall_avg_uo_name = genName(name(), "T_wall_avg_uo");
  {
    const std::string class_name = "LayeredSideAverage";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") = {HeatConductionModel::TEMPERATURE};
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    params.set<MooseEnum>("direction") = _layered_average_uo_direction;
    params.set<unsigned int>("num_layers") = flow_channel.getNumElems();
    _sim.addUserObject(class_name, T_wall_avg_uo_name, params);
  }
  {
    std::string class_name = "WallTemperatureFromHS3DAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    params.set<UserObjectName>("T_wall_avg_uo") = T_wall_avg_uo_name;
    _sim.addAuxKernel(class_name, genName(name(), "T_wall_transfer"), params);
  }

  const UserObjectName heat_transfer_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "HeatTransferFromHeatStructure3D1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<FlowChannel3DAlignment *>("_fch_alignment") = &_fch_alignment;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, heat_transfer_uo_name, params);
  }

  {
    const std::string class_name = "OneD3EqnEnergyHeatFluxFromHeatStructure3D";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<UserObjectName>("T_wall_avg_uo") = T_wall_avg_uo_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    _sim.addKernel(class_name, genName(name(), "heat_flux_kernel"), params);
  }

  {
    const std::string class_name = "ConvectionHeatTransfer3DBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<UserObjectName>("ht_uo") = heat_transfer_uo_name;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("T_wall") = {HeatConductionModel::TEMPERATURE};
    _sim.addBoundaryCondition(class_name, genName(name(), "heat_flux_bc"), params);
  }
}
