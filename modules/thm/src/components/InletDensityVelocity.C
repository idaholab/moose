#include "InletDensityVelocity.h"

registerMooseObject("THMApp", InletDensityVelocity);

template <>
InputParameters
validParams<InletDensityVelocity>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("rho", "Prescribed density");
  params.addParam<Real>("vel", "Prescribed velocity");
  // 2-phase
  params.addParam<Real>("rho_liquid", "Prescribed density of liquid");
  params.addParam<Real>("vel_liquid", "Prescribed velocity of liquid");
  params.addParam<Real>("rho_vapor", "Prescribed density of vapor");
  params.addParam<Real>("vel_vapor", "Prescribed velocity of vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletDensityVelocity::InletDensityVelocity(const InputParameters & params) : FlowBoundary(params) {}

void
InletDensityVelocity::check() const
{
  FlowBoundary::check();

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
    logError("InletDensityVelocity component is deprecated. Use 'type = "
             "InletDensityVelocity1Phase' instead.");
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
    logError("InletDensityVelocity component is deprecated. Use 'type = "
             "InletDensityVelocity2Phase' instead.");
}
