#include "HeatTransferFromSpecifiedTemperature1Phase.h"

registerMooseObject("THMApp", HeatTransferFromSpecifiedTemperature1Phase);

template <>
InputParameters
validParams<HeatTransferFromSpecifiedTemperature1Phase>()
{
  InputParameters params = validParams<HeatTransferFromTemperature1Phase>();
  params.addRequiredParam<FunctionName>("T_wall", "Specified wall temperature function");
  params.addClassDescription(
      "Heat transfer connection from a fixed temperature function for 1-phase flow");
  return params;
}

HeatTransferFromSpecifiedTemperature1Phase::HeatTransferFromSpecifiedTemperature1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters), _T_wall_fn_name(getParam<FunctionName>("T_wall"))
{
}

void
HeatTransferFromSpecifiedTemperature1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  _sim.addFunctionIC(_T_wall_name, _T_wall_fn_name, _flow_channel_name);
  makeFunctionControllableIfConstant(_T_wall_fn_name, "T_wall");
}

void
HeatTransferFromSpecifiedTemperature1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  {
    const std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<std::vector<SubdomainName>>("block") = {_flow_channel_name};
    params.set<FunctionName>("function") = _T_wall_fn_name;

    ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
    execute_on = {EXEC_INITIAL, EXEC_LINEAR};
    params.set<ExecFlagEnum>("execute_on") = execute_on;

    _sim.addAuxKernel(class_name, genName(name(), "T_wall_auxkernel"), params);
  }

  addHeatTransferKernels();
}
