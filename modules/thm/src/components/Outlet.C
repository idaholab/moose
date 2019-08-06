#include "Outlet.h"

registerMooseObject("THMApp", Outlet);

template <>
InputParameters
validParams<Outlet>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addParam<bool>(
      "reversible",
      false,
      "true for reversible outlet boundary conditions (works only for 7-eqn model)");
  // single phase
  params.addParam<Real>("p", "prescribed pressure");
  // two phase
  params.addParam<Real>("p_liquid", "Prescribed pressure for the liquid phase");
  params.addParam<Real>("p_vapor", "Prescribed pressure for the vapor phase");

  params.addParam<bool>(
      "legacy", false, "Use the old version of the BC (violating characteristic theory) or not.");

  return params;
}

Outlet::Outlet(const InputParameters & params) : FlowBoundary(params) {}

void
Outlet::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    logError("Outlet component is deprecated. Use 'type = Outlet1Phase' instead.");
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    logError("Outlet component is deprecated. Use 'type = Outlet2Phase' instead.");
}
