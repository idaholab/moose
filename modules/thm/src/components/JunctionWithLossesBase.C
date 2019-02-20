#include "JunctionWithLossesBase.h"

template <>
InputParameters
validParams<JunctionWithLossesBase>()
{
  InputParameters params = validParams<FlowJunction>();
  params.addRequiredParam<std::vector<Real>>("K", "Form loss coefficients");
  // use same values in K for K_reverse if not provided
  params.addParam<std::vector<Real>>("K_reverse", "Reverse form loss coefficients");
  params.addRequiredParam<Real>("A_ref", "Reference area of this junction");

  return params;
}

JunctionWithLossesBase::JunctionWithLossesBase(const InputParameters & parameters)
  : FlowJunction(parameters),
    _k_coeffs(getParam<std::vector<Real>>("K")),
    _kr_coeffs(isParamValid("K_reverse") ? getParam<std::vector<Real>>("K_reverse")
                                         : getParam<std::vector<Real>>("K")),
    _ref_area(getParam<Real>("A_ref"))
{
  checkSizeEqualsNumberOfConnections<Real>("K");
  if (isParamValid("K_reverse"))
    checkSizeEqualsNumberOfConnections<Real>("K_reverse");
}
