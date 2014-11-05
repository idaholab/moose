//  Hardening model base class.
//
#include "TensorMechanicsHardeningConstant.h"

template<>
InputParameters validParams<TensorMechanicsHardeningConstant>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addParam<Real>("value", "The value of the parameter for all internal parameter(s)");
  params.addClassDescription("No hardening - the parameter is independent of the internal parameter(s)");
  return params;
}

TensorMechanicsHardeningConstant::TensorMechanicsHardeningConstant(const std::string & name, InputParameters parameters) :
  TensorMechanicsHardeningModel(name, parameters),
  _val(getParam<Real>("value"))
{
}

Real
TensorMechanicsHardeningConstant::value(const Real & /*intnl*/) const
{
  return _val;
}

Real
TensorMechanicsHardeningConstant::derivative(const Real & /*intnl*/) const
{
  return 0.0;
}
