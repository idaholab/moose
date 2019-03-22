#include "HeatTransfer1PhaseBase.h"
#include "InputParameterLogic.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<HeatTransfer1PhaseBase>()
{
  InputParameters params = validParams<HeatTransferBase>();
  params.addParam<FunctionName>("Hw", "Convective heat transfer coefficient");
  return params;
}

HeatTransfer1PhaseBase::HeatTransfer1PhaseBase(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransfer1PhaseBase::init()
{
  HeatTransferBase::init();
}

void
HeatTransfer1PhaseBase::initSecondary()
{
  HeatTransferBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<FlowChannel1Phase>(_pipe_name))
  {
    const FlowChannel1Phase & pipe = getComponentByName<FlowChannel1Phase>(_pipe_name);

    const std::string suffix = pipe.getHeatTransferNamesSuffix(name());
    const std::string Hw_suffix = _closures_name == "simple" ? suffix : "";

    _Hw_1phase_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + Hw_suffix;
  }
}

void
HeatTransfer1PhaseBase::check() const
{
  HeatTransferBase::check();

  if (_closures_name == "simple")
  {
    if (!isParamValid("Hw"))
      logError("The parameter 'Hw' must be provided when using simple closures.");
  }
  else if (_closures_name == "trace")
  {
    if (isParamValid("Hw"))
      logError("The parameter 'Hw' cannot be provided when using TRACE closures.");
  }
}

void
HeatTransfer1PhaseBase::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  if (_closures_name == "simple")
  {
    const FunctionName & Hw_fn_name = getParam<FunctionName>("Hw");

    {
      const std::string class_name = "GenericFunctionMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
      params.set<std::vector<std::string>>("prop_names") = {_Hw_1phase_name};
      params.set<std::vector<FunctionName>>("prop_values") = {Hw_fn_name};
      _sim.addMaterial(class_name, genName(name(), "Hw_material"), params);
    }

    makeFunctionControllableIfConstant(Hw_fn_name, "Hw");
  }
}

const MaterialPropertyName &
HeatTransfer1PhaseBase::getWallHeatTransferCoefficient1PhaseName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_1phase_name;
}
