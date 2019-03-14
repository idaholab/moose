#include "HeatTransferBase.h"
#include "InputParameterLogic.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowChannel.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<HeatTransferBase>()
{
  InputParameters params = validParams<ConnectorBase>();
  params.addRequiredParam<std::string>("pipe", "Name of pipe component to connect");
  params.addParam<bool>(
      "P_hf_transferred", false, "Is heat flux perimeter transferred from an external source?");
  params.addParam<FunctionName>("P_hf", "Heat flux perimeter function");
  return params;
}

HeatTransferBase::HeatTransferBase(const InputParameters & parameters)
  : ConnectorBase(parameters),
    _pipe_name(getParam<std::string>("pipe")),
    _P_hf_transferred(getParam<bool>("P_hf_transferred")),
    _P_hf_provided(isParamValid("P_hf"))
{
  addDependency(_pipe_name);
}

void
HeatTransferBase::init()
{
  ConnectorBase::init();

  checkComponentOfTypeExistsByName<FlowChannel>(_pipe_name);

  if (hasComponentByName<FlowChannel>(_pipe_name))
  {
    const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);

    // add the name of this heat transfer component to list for pipe
    pipe.addHeatTransferName(name());

    // get various data from pipe
    _block_ids_pipe = pipe.getSubdomainIds();
    _model_type = pipe.getFlowModelID();
    _fp_name = pipe.getFluidPropertiesName();
    _A_fn_name = pipe.getAreaFunctionName();
    _closures_name = MooseUtils::toLower(pipe.getParam<std::string>("closures"));
  }
}

void
HeatTransferBase::initSecondary()
{
  ConnectorBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<FlowChannel>(_pipe_name))
  {
    const FlowChannel & pipe = getComponentByName<FlowChannel>(_pipe_name);

    const std::string suffix = pipe.getHeatTransferNamesSuffix(name());

    _P_hf_name = FlowModel::HEAT_FLUX_PERIMETER + suffix;
    _T_wall_name = FlowModel::TEMPERATURE_WALL + suffix;
    _q_wall_name = FlowModel::HEAT_FLUX_WALL + suffix;
  }
}

void
HeatTransferBase::check() const
{
  ConnectorBase::check();
}

void
HeatTransferBase::addVariables()
{
  // heat flux perimeter variable
  if (!_P_hf_transferred)
    addHeatedPerimeter();
}

void
HeatTransferBase::addMooseObjects()
{
  // create heat flux perimeter aux if not transferred from external app
  if (!_P_hf_transferred)
  {
    const std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = {_P_hf_name};
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<FunctionName>("function") = _P_hf_fn_name;

    ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
    execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};
    params.set<ExecFlagEnum>("execute_on") = execute_on;

    _sim.addAuxKernel(class_name, genName(name(), "P_hf_auxkernel"), params);
  }
}

void
HeatTransferBase::addHeatedPerimeter()
{
  _sim.addVariable(false, _P_hf_name, FlowModel::feType(), _block_ids_pipe);

  // create heat flux perimeter variable if not transferred from external app
  if (!_P_hf_transferred)
  {
    if (_P_hf_provided)
    {
      _P_hf_fn_name = getParam<FunctionName>("P_hf");
    }
    // create heat flux perimeter function if not provided; assume circular pipe
    else
    {
      _P_hf_fn_name = genName(name(), "P_hf_fn");

      const std::string class_name = "GeneralizedCircumference";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<FunctionName>("area_function") = _A_fn_name;
      _sim.addFunction(class_name, _P_hf_fn_name, params);

      makeFunctionControllableIfConstant(_P_hf_fn_name, "P_hf");
    }

    _sim.addFunctionIC(_P_hf_name, _P_hf_fn_name, _pipe_name);
  }
}

const VariableName &
HeatTransferBase::getHeatedPerimeterName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _P_hf_name;
}

const VariableName &
HeatTransferBase::getWallTemperatureName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _T_wall_name;
}

const VariableName &
HeatTransferBase::getWallHeatFluxName() const
{
  return _q_wall_name;
}
