#include "HeatTransferFromHeatFlux.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowChannel.h"

registerMooseObject("THMApp", HeatTransferFromHeatFlux);

template <>
InputParameters
validParams<HeatTransferFromHeatFlux>()
{
  InputParameters params = validParams<HeatTransferBase>();

  params.addRequiredParam<FunctionName>("q_wall", "Specified wall heat flux function");

  return params;
}

HeatTransferFromHeatFlux::HeatTransferFromHeatFlux(const InputParameters & parameters)
  : HeatTransferBase(parameters), _q_wall_fn_name(getParam<FunctionName>("q_wall"))
{
}

void
HeatTransferFromHeatFlux::addVariables()
{
  HeatTransferBase::addVariables();

  _sim.addVariable(false, _q_wall_name, FlowModel::feType(), _block_ids_pipe);
}

void
HeatTransferFromHeatFlux::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  {
    const std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = {_q_wall_name};
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<FunctionName>("function") = _q_wall_fn_name;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    _sim.addAuxKernel(class_name, genName(name(), "q_wall_auxkernel"), params);
  }

  if (_model_type == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    addMooseObjects2Phase();
}

void
HeatTransferFromHeatFlux::addMooseObjects1Phase()
{
  // wall heat transfer kernel
  {
    const std::string class_name = "OneDEnergyWallHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<std::vector<VariableName>>("q_wall") = {_q_wall_name};
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    _sim.addKernel(class_name, genName(name(), "wall_heat"), params);
  }
}

void
HeatTransferFromHeatFlux::addMooseObjects2Phase()
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

  // wall heat transfer kernel
  for (unsigned int k = 0; k < 2; k++)
  {
    const std::string class_name = "OneD7EqnEnergyWallHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = arhoEA_name[k];
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<bool>("is_liquid") = is_liquid[k];
    params.set<std::vector<VariableName>>("q_wall") = {_q_wall_name};
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    params.set<MaterialPropertyName>("alpha") = alpha_name[k];
    params.set<MaterialPropertyName>("heat_flux_partitioning_liquid") =
        FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;

    _sim.addKernel(class_name, genName(name(), "wall_heat", phase_name[k]), params);
  }

  // wall boiling kernel
  const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);
  if (pipe.getParam<bool>("wall_mass_transfer"))
  {
    for (unsigned int k = 0; k < 2; k++)
    {
      const std::string class_name = "OneD7EqnEnergyWallBoilingInterfaceHeatFlux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = arhoEA_name[k];
      params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
      params.set<bool>("is_liquid") = is_liquid[k];
      params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      params.set<std::vector<VariableName>>("arhoA_liquid") = {arhoA_name[0]};
      params.set<std::vector<VariableName>>("arhouA_liquid") = {arhouA_name[0]};
      params.set<std::vector<VariableName>>("arhoEA_liquid") = {arhoEA_name[0]};
      params.set<MaterialPropertyName>("alpha_liquid") = alpha_name[0];
      params.set<std::vector<VariableName>>("q_wall") = {_q_wall_name};
      params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
      params.set<MaterialPropertyName>("kappa_liquid") =
          FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;
      params.set<MaterialPropertyName>("wall_boiling_fraction") =
          FlowModelTwoPhase::WALL_BOILING_FRACTION;

      _sim.addKernel(
          class_name, genName(name(), "wall_boiling_interface_heat", phase_name[k]), params);
    }
  }
}

bool
HeatTransferFromHeatFlux::isTemperatureType() const
{
  return false;
}
