#include "HeatTransferFromHeatStructure.h"
#include "FlowChannel.h"
#include "HeatStructure.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure);

template <>
InputParameters
validParams<HeatTransferFromHeatStructure>()
{
  InputParameters params = validParams<HeatTransferFromTemperature>();

  params.addRequiredParam<std::string>("hs", "The name of the heat structure component");

  MooseEnum hs_sides("top bottom");
  params.addRequiredParam<MooseEnum>("hs_side", hs_sides, "The side of the heat structure");

  return params;
}

HeatTransferFromHeatStructure::HeatTransferFromHeatStructure(const InputParameters & parameters)
  : HeatTransferFromTemperature(parameters),
    _hs_name(getParam<std::string>("hs")),
    _hs_side(getParam<MooseEnum>("hs_side"))
{
}

void
HeatTransferFromHeatStructure::check() const
{
  HeatTransferFromTemperature::check();

  checkComponentOfTypeExistsByName<HeatStructure>(_hs_name);

  if (hasComponentByName<HeatStructure>(_hs_name))
  {
    const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
    if (hs.getDimension() == 1)
      logError("1D heat structures cannot be used; 2D heat structures must be used instead.");
  }
}

void
HeatTransferFromHeatStructure::addVariables()
{
  HeatTransferFromTemperature::addVariables();

  // wall temperature initial condition
  if (!_app.isRestarting())
  {
    const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
    _sim.addFunctionIC(_T_wall_name, hs.getInitialT(), _pipe_name);
  }
}

void
HeatTransferFromHeatStructure::addMooseObjects()
{
  HeatTransferFromTemperature::addMooseObjects();

  if (_model_type == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    addMooseObjects2Phase();

  // Transfer the temperature of the solid onto the pipe
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

void
HeatTransferFromHeatStructure::addMooseObjects1Phase()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
  const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);

  const UserObjectName heat_flux_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "HeatFluxFromHeatStructure3EqnUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = pipe.getSubdomainNames();
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
    params.set<std::vector<SubdomainName>>("block") = pipe.getSubdomainNames();
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
}

void
HeatTransferFromHeatStructure::addMooseObjects2Phase()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructure & hs = getComponentByName<HeatStructure>(_hs_name);
  const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);

  std::vector<bool> is_liquid{true, false};
  std::vector<std::string> phase{"liquid", "vapor"};
  std::vector<VariableName> arhoA_name{FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                       FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
  std::vector<VariableName> arhouA_name{FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                        FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
  std::vector<VariableName> arhoEA_name{FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                        FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
  std::vector<VariableName> T_name{FlowModelTwoPhase::TEMPERATURE_LIQUID,
                                   FlowModelTwoPhase::TEMPERATURE_VAPOR};
  std::vector<MaterialPropertyName> Hw_name{_Hw_liquid_name, _Hw_vapor_name};

  for (unsigned int k = 0; k < 2; k++)
  {
    const UserObjectName heat_flux_uo_name = genName(name(), "heat_flux_uo", phase[k]);
    {
      const std::string class_name = "HeatFluxFromHeatStructure7EqnUserObject";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = pipe.getSubdomainNames();
      params.set<bool>("is_liquid") = is_liquid[k];
      params.set<std::vector<BoundaryName>>("slave_boundary") = {getSlaveSideName()};
      params.set<BoundaryName>("master_boundary") = getMasterSideName();
      params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
      params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
      params.set<MaterialPropertyName>("Hw") = Hw_name[k];
      params.set<MaterialPropertyName>("T") = T_name[k];
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA") = {arhoA_name[k]};
      params.set<std::vector<VariableName>>("arhouA") = {arhouA_name[k]};
      params.set<std::vector<VariableName>>("arhoEA") = {arhoEA_name[k]};
      params.set<MaterialPropertyName>("heat_flux_partitioning_liquid") =
          FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;
      params.set<ExecFlagEnum>("execute_on") = execute_on;
      _sim.addUserObject(class_name, heat_flux_uo_name, params);
    }

    {
      const std::string class_name = "OneD7EqnEnergyHeatFlux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = pipe.getSubdomainNames();
      params.set<NonlinearVariableName>("variable") = arhoEA_name[k];
      params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA") = {arhoA_name[k]};
      params.set<std::vector<VariableName>>("arhouA") = {arhouA_name[k]};
      params.set<std::vector<VariableName>>("arhoEA") = {arhoEA_name[k]};
      params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
      _sim.addKernel(class_name, genName(name(), "heat_flux_kernel", phase[k]), params);
    }

    {
      const std::string class_name = "HeatFlux7EqnBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {getMasterSideName()};
      params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
      params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
      params.set<Real>("P_hs_unit") = hs.getUnitPerimeter(_hs_side);
      params.set<unsigned int>("n_unit") = hs.getNumberOfUnits();
      params.set<bool>("hs_coord_system_is_cylindrical") = hs.isCylindrical();
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA") = {arhoA_name[k]};
      params.set<std::vector<VariableName>>("arhouA") = {arhouA_name[k]};
      params.set<std::vector<VariableName>>("arhoEA") = {arhoEA_name[k]};
      params.set<std::vector<VariableName>>("T_wall") = {HeatConductionModel::TEMPERATURE};
      _sim.addBoundaryCondition(class_name, genName(name(), "heat_flux_bc", phase[k]), params);
    }
  }
}

const BoundaryName &
HeatTransferFromHeatStructure::getMasterSideName() const
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
HeatTransferFromHeatStructure::getSlaveSideName() const
{
  const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);
  return pipe.getNodesetName();
}
