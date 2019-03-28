#include "HeatTransferFromHeatStructure1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructure.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure1Phase);

template <>
InputParameters
validParams<HeatTransferFromHeatStructure1Phase>()
{
  InputParameters params = validParams<HeatTransferFromTemperature1Phase>();
  params.addRequiredParam<std::string>("hs", "The name of the heat structure component");
  MooseEnum hs_sides("top bottom");
  params.addRequiredParam<MooseEnum>("hs_side", hs_sides, "The side of the heat structure");
  params.addClassDescription("Connects a 1-phase flow channel and a heat structure");
  return params;
}

HeatTransferFromHeatStructure1Phase::HeatTransferFromHeatStructure1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters),
    _hs_name(getParam<std::string>("hs")),
    _hs_side(getParam<MooseEnum>("hs_side"))
{
}

void
HeatTransferFromHeatStructure1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();

  checkComponentOfTypeExistsByName<HeatStructure>(_hs_name);

  if (hasComponentByName<HeatStructure>(_hs_name))
  {
    const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
    if (hs.getDimension() == 1)
      logError("1D heat structures cannot be used; 2D heat structures must be used instead.");
  }
}

void
HeatTransferFromHeatStructure1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  // wall temperature initial condition
  if (!_app.isRestarting())
  {
    const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
    _sim.addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_name);
  }
}

void
HeatTransferFromHeatStructure1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
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
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
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
    params.set<bool>("hs_coord_system_is_cylindrical") = hs.isCylindrical();
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
  const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);

  switch (_hs_side)
  {
    case 0:
      return hs.getTopBoundaryNames()[0];
    case 1:
      return hs.getBottomBoundaryNames()[0];
    default:
      mooseError(name(), ": Unknown side specified in the 'hs_side' parameter.");
  }
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getSlaveSideName() const
{
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);
  return flow_channel.getNodesetName();
}
