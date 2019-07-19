#include "InletMassFlowRateTemperature.h"

registerMooseObject("THMApp", InletMassFlowRateTemperature);

template <>
InputParameters
validParams<InletMassFlowRateTemperature>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("m_dot", "Prescribed mass flow rate");
  params.addParam<Real>("T", "prescribed temperature (used only in 3eqn model)");
  // 2-phase
  params.addParam<Real>("m_dot_liquid", "Prescribed mass flow rate of liquid");
  params.addParam<Real>("T_liquid", "Prescribed temperature of liquid");
  params.addParam<Real>("m_dot_vapor", "Prescribed mass flow rate of vapor");
  params.addParam<Real>("T_vapor", "Prescribed temperature of vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletMassFlowRateTemperature::InletMassFlowRateTemperature(const InputParameters & params)
  : FlowBoundary(params)
{
}

void
InletMassFlowRateTemperature::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    logError("InletMassFlowRateTemperature component is deprecated. Use 'type = "
             "InletMassFlowRateTemperature1Phase' instead.");
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    logError("InletMassFlowRateTemperature component is deprecated. Use 'type = "
             "InletMassFlowRateTemperature2Phase' instead.");
}
