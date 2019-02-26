#include "HeatTransferFromTemperature.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "HeatConductionModel.h"
#include "FlowChannel.h"

template <>
InputParameters
validParams<HeatTransferFromTemperature>()
{
  InputParameters params = validParams<HeatTransferBase>();
  return params;
}

HeatTransferFromTemperature::HeatTransferFromTemperature(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransferFromTemperature::addVariables()
{
  HeatTransferBase::addVariables();

  _sim.addVariable(
      false, FlowModel::TEMPERATURE_WALL, HeatConductionModel::feType(), _block_ids_pipe);
  _sim.addVariable(false, _T_wall_name, HeatConductionModel::feType(), _block_ids_pipe);
}

void
HeatTransferFromTemperature::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  // wall boiling interface heat transfer
  const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);
  if (_model_type == THM::FM_TWO_PHASE && pipe.getParam<bool>("wall_mass_transfer"))
  {
    const std::vector<bool> is_liquid{true, false};
    const std::vector<std::string> phase_name{"liquid", "vapor"};
    const std::vector<VariableName> arhoEA_name{FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                                FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};

    for (unsigned int k = 0; k < 2; ++k)
    {
      const std::string class_name = "OneD7EqnEnergyWallBoilingInterfaceHeatTransfer";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = arhoEA_name[k];
      params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
      params.set<bool>("is_liquid") = is_liquid[k];
      params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
      params.set<MaterialPropertyName>("Hw_liquid") = {_Hw_liquid_name};
      params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHO_A_LIQUID};
      params.set<std::vector<VariableName>>("arhouA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID};
      params.set<std::vector<VariableName>>("arhoEA_liquid") = {arhoEA_name[0]};
      params.set<MaterialPropertyName>("T_liquid") = FlowModelTwoPhase::TEMPERATURE_LIQUID;
      params.set<MaterialPropertyName>("alpha_liquid") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
      params.set<MaterialPropertyName>("kappa_liquid") =
          FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;
      params.set<MaterialPropertyName>("wall_boiling_fraction") =
          FlowModelTwoPhase::WALL_BOILING_FRACTION;

      _sim.addKernel(class_name,
                     genName(name(), "wall_boiling_interface_heat_transfer", phase_name[k]),
                     params);
    }
  }
}

void
HeatTransferFromTemperature::addHeatTransferKernels1Phase()
{
  {
    const std::string class_name = "OneDEnergyWallHeating";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    _sim.addKernel(class_name, genName(name(), "wall_heat_transfer"), params);
  }
}

void
HeatTransferFromTemperature::addHeatTransferKernels2Phase()
{
  const std::vector<bool> is_liquid{true, false};
  const std::vector<std::string> phase_name{"liquid", "vapor"};
  const std::vector<VariableName> arhoA_name{FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                             FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
  const std::vector<VariableName> arhouA_name{FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                              FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
  const std::vector<VariableName> arhoEA_name{FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                              FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
  const std::vector<MaterialPropertyName> alpha_name{FlowModelTwoPhase::VOLUME_FRACTION_LIQUID,
                                                     FlowModelTwoPhase::VOLUME_FRACTION_VAPOR};
  const std::vector<MaterialPropertyName> T_name{FlowModelTwoPhase::TEMPERATURE_LIQUID,
                                                 FlowModelTwoPhase::TEMPERATURE_VAPOR};
  const std::vector<VariableName> Hw_name{_Hw_liquid_name, _Hw_vapor_name};

  for (unsigned int k = 0; k < 2; ++k)
  {
    const std::string class_name = "OneD7EqnEnergyWallHeatTransfer";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = arhoEA_name[k];
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<bool>("is_liquid") = is_liquid[k];
    params.set<MaterialPropertyName>("alpha") = alpha_name[k];
    params.set<MaterialPropertyName>("T") = T_name[k];
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<MaterialPropertyName>("Hw") = {Hw_name[k]};
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    params.set<std::vector<VariableName>>("arhoA") = {arhoA_name[k]};
    params.set<std::vector<VariableName>>("arhouA") = {arhouA_name[k]};
    params.set<std::vector<VariableName>>("arhoEA") = {arhoEA_name[k]};
    params.set<MaterialPropertyName>("heat_flux_partitioning_liquid") =
        FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;

    _sim.addKernel(class_name, genName(name(), "wall_heat_transfer", phase_name[k]), params);
  }
}

bool
HeatTransferFromTemperature::isTemperatureType() const
{
  return true;
}
