#include "SupersonicInlet.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", SupersonicInlet);

InputParameters
SupersonicInlet::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addParam<Real>("p", "Prescribed pressure [Pa]");
  params.addParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<Real>("vel", "Prescribed velocity [m/s]");
  return params;
}

SupersonicInlet::SupersonicInlet(const InputParameters & parameters)
  : FlowBoundary1Phase(parameters)
{
}

void
SupersonicInlet::check() const
{
  FlowBoundary1Phase::check();

  logModelNotImplementedError(_flow_model_id);
}

void
SupersonicInlet::addMooseObjects()
{
}
