#include "HeatTransferFromSpecifiedTemperature.h"

registerMooseObject("THMApp", HeatTransferFromSpecifiedTemperature);

template <>
InputParameters
validParams<HeatTransferFromSpecifiedTemperature>()
{
  InputParameters params = validParams<HeatTransferFromTemperature>();

  params.addRequiredParam<FunctionName>("T_wall", "Specified wall temperature function");

  return params;
}

HeatTransferFromSpecifiedTemperature::HeatTransferFromSpecifiedTemperature(
    const InputParameters & parameters)
  : HeatTransferFromTemperature(parameters), _T_wall_fn_name(getParam<FunctionName>("T_wall"))
{
}

void
HeatTransferFromSpecifiedTemperature::addVariables()
{
  HeatTransferFromTemperature::addVariables();

  _sim.addFunctionIC(_T_wall_name, _T_wall_fn_name, _pipe_name);
  makeFunctionControllableIfConstant(_T_wall_fn_name, "T_wall");
}

void
HeatTransferFromSpecifiedTemperature::addMooseObjects()
{
  HeatTransferFromTemperature::addMooseObjects();

  {
    const std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<FunctionName>("function") = _T_wall_fn_name;

    ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
    execute_on = {EXEC_INITIAL, EXEC_LINEAR};
    params.set<ExecFlagEnum>("execute_on") = execute_on;

    _sim.addAuxKernel(class_name, genName(name(), "T_wall_auxkernel"), params);
  }

  if (_model_type == THM::FM_SINGLE_PHASE)
    addHeatTransferKernels1Phase();
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    addHeatTransferKernels2Phase();
}
