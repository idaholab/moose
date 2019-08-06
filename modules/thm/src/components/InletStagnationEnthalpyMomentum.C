#include "InletStagnationEnthalpyMomentum.h"

registerMooseObject("THMApp", InletStagnationEnthalpyMomentum);

template <>
InputParameters
validParams<InletStagnationEnthalpyMomentum>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("rhou", "Prescribed momentum");
  params.addParam<Real>("H", "Prescribed specific total enthalpy");
  // 2-phase
  params.addParam<Real>("rhou_liquid", "Prescribed momentum for liquid");
  params.addParam<Real>("rhou_vapor", "Prescribed momentum for vapor");
  params.addParam<Real>("H_liquid", "Prescribed specific total enthalpy for liquid");
  params.addParam<Real>("H_vapor", "Prescribed specific total enthalpy for vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletStagnationEnthalpyMomentum::InletStagnationEnthalpyMomentum(const InputParameters & params)
  : FlowBoundary(params)
{
}

void
InletStagnationEnthalpyMomentum::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    logError("InletStagnationEnthalpyMomentum component is deprecated. Use 'type = "
             "InletStagnationEnthalpyMomentum1Phase' instead.");
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    logError("InletStagnationEnthalpyMomentum component is deprecated. Use 'type = "
             "InletStagnationEnthalpyMomentum2Phase' instead.");
}
