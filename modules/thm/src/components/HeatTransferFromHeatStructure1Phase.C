#include "HeatTransferFromHeatStructure1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindrical.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"
#include "FlowChannelAlignment.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure1Phase);

template <>
InputParameters
validParams<HeatTransferFromHeatStructure1Phase>()
{
  InputParameters params = validParams<HeatTransferFromTemperature1Phase>();
  params += validParams<HSBoundaryInterface>();

  params.addClassDescription("Connects a 1-phase flow channel and a heat structure");

  return params;
}

HeatTransferFromHeatStructure1Phase::HeatTransferFromHeatStructure1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters), HSBoundaryInterface(this)
{
}

void
HeatTransferFromHeatStructure1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();
  HSBoundaryInterface::check(this);

  if (hasComponentByName<HeatStructureBase>(_hs_name) &&
      hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);

    if (hs.getNumElems() != flow_channel.getNumElems())
      logError("The number of elements in component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getNumElems(),
               ", but the number of axial elements in component '",
               _hs_name,
               "' is ",
               hs.getNumElems(),
               ". They must be the same.");

    if (hs.getLength() != flow_channel.getLength())
      logError("The length of component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getLength(),
               ", but the length of component '",
               _hs_name,
               "' is ",
               hs.getLength(),
               ". They must be the same.");

    if (_hs_side_valid)
    {
      FlowChannelAlignment fcha(flow_channel, getMasterSideName(), getSlaveSideName());
      fcha.build();
      if (!fcha.check())
        logError("The centers of the elements of flow channel '",
                 _flow_channel_name,
                 "' do not equal the centers of the specified heat structure side.");
    }
  }
}

void
HeatTransferFromHeatStructure1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  // wall temperature initial condition
  if (!_app.isRestarting())
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    _sim.addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_name);
  }
}

void
HeatTransferFromHeatStructure1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
  const bool is_cylindrical = dynamic_cast<const HeatStructureCylindrical *>(&hs) != nullptr;
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  const UserObjectName heat_flux_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "HeatFluxFromHeatStructure3EqnUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::vector<BoundaryName>>("slave_boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("master_boundary") = getMasterSideName();
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, heat_flux_uo_name, params);
  }

  {
    const std::string class_name = "OneD3EqnEnergyHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    _sim.addKernel(class_name, genName(name(), "heat_flux_kernel"), params);
  }

  {
    const std::string class_name = "HeatFlux3EqnBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {getMasterSideName()};
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    params.set<Real>("P_hs_unit") = hs.getUnitPerimeter(_hs_side);
    params.set<unsigned int>("n_unit") = hs.getNumberOfUnits();
    params.set<bool>("hs_coord_system_is_cylindrical") = is_cylindrical;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("T_wall") = {HeatConductionModel::TEMPERATURE};
    _sim.addBoundaryCondition(class_name, genName(name(), "heat_flux_bc"), params);
  }

  // Transfer the temperature of the solid onto the flow channel
  {
    std::string class_name = "VariableValueTransferAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<std::vector<BoundaryName>>("boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("paired_boundary") = getMasterSideName();
    params.set<std::vector<VariableName>>("paired_variable") =
        std::vector<VariableName>(1, HeatConductionModel::TEMPERATURE);
    _sim.addAuxBoundaryCondition(class_name, genName(name(), "T_wall_transfer"), params);
  }
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getMasterSideName() const
{
  return getHSBoundaryName(this);
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getSlaveSideName() const
{
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);
  return flow_channel.getNodesetName();
}
