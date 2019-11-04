#include "LinearTest7EqnFluidProperties.h"

registerMooseObject("THMTestApp", LinearTest7EqnFluidProperties);

template <>
InputParameters
validParams<LinearTest7EqnFluidProperties>()
{
  InputParameters params = validParams<TwoPhaseFluidProperties>();

  params.addParam<Real>("dT_sat_dp", 2, "Derivative of saturation temperature w.r.t. pressure");
  params.addParam<Real>(
      "drho_int_dp_int", 3, "Derivative of interfacial density w.r.t. interfacial pressure");
  params.addParam<Real>(
      "drho_int_dT_int", 4, "Derivative of interfacial density w.r.t. interfacial temperature");
  params.addParam<Real>("dsigma_dT", 5, "Derivative of surface tension w.r.t. temperature");

  return params;
}

LinearTest7EqnFluidProperties::LinearTest7EqnFluidProperties(const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters),
    _dT_sat_dp(getParam<Real>("dT_sat_dp")),
    _drho_int_dp_int(getParam<Real>("drho_int_dp_int")),
    _drho_int_dT_int(getParam<Real>("drho_int_dT_int")),
    _dsigma_dT(getParam<Real>("dsigma_dT"))
{
}

Real
LinearTest7EqnFluidProperties::p_critical() const
{
  return 0;
}

Real
LinearTest7EqnFluidProperties::T_sat(Real p) const
{
  return _dT_sat_dp * p;
}

Real LinearTest7EqnFluidProperties::p_sat(Real) const { return 0; }

Real LinearTest7EqnFluidProperties::dT_sat_dp(Real) const { return _dT_sat_dp; }

Real
LinearTest7EqnFluidProperties::sigma_from_T(Real T) const
{
  return _dsigma_dT * T;
}

Real LinearTest7EqnFluidProperties::dsigma_dT_from_T(Real /*T*/) const { return _dsigma_dT; }
