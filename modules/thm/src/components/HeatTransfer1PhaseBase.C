#include "HeatTransfer1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"
#include "ClosuresBase.h"
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
  if (hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);
    const std::string Hw_suffix = flow_channel.getHeatTransferNamesSuffix(name());

    _Hw_1phase_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + Hw_suffix;
  }
}

void
HeatTransfer1PhaseBase::check() const
{
  HeatTransferBase::check();

  if (_closures != nullptr)
    _closures->check(*this);
}

void
HeatTransfer1PhaseBase::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  _closures->addMooseObjects(*this);
}

const MaterialPropertyName &
HeatTransfer1PhaseBase::getWallHeatTransferCoefficient1PhaseName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_1phase_name;
}
