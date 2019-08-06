#include "InletStagnationPressureTemperature.h"

registerMooseObject("THMApp", InletStagnationPressureTemperature);

template <>
InputParameters
validParams<InletStagnationPressureTemperature>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("p0", "Prescribed stagnation pressure");
  params.addParam<Real>("T0", "Prescribed stagnation temperature");
  // 2-phase
  params.addParam<Real>("p0_liquid", "Prescribed stagnation pressure for liquid phase");
  params.addParam<Real>("T0_liquid", "Prescribed stagnation temperature for liquid phase");
  params.addParam<Real>("p0_vapor", "Prescribed stagnation pressure for vapor phase");
  params.addParam<Real>("T0_vapor", "Prescribed stagnation temperature for vapor phase");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletStagnationPressureTemperature::InletStagnationPressureTemperature(
    const InputParameters & params)
  : FlowBoundary(params)
{
}

void
InletStagnationPressureTemperature::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    logError("InletStagnationPressureTemperature component is deprecated. Use 'type = "
             "InletStagnationPressureTemperature1Phase' instead.");
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    logError("InletStagnationPressureTemperature component is deprecated. Use 'type = "
             "InletStagnationPressureTemperature2Phase' instead.");
}
