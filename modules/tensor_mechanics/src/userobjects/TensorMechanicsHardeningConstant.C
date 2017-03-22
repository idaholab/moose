/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsHardeningConstant.h"
#include <math.h> // for M_PI

template <>
InputParameters
validParams<TensorMechanicsHardeningConstant>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addParam<Real>("value",
                        1.0,
                        "The value of the parameter for all internal parameter.  "
                        "This is perfect plasticity - there is no hardening.");
  params.addParam<bool>("convert_to_radians",
                        false,
                        "If true, the value you entered will be "
                        "multiplied by Pi/180 before passing to the "
                        "Plasticity algorithms");
  params.addClassDescription(
      "No hardening - the parameter is independent of the internal parameter(s)");
  return params;
}

TensorMechanicsHardeningConstant::TensorMechanicsHardeningConstant(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val(getParam<bool>("convert_to_radians") ? getParam<Real>("value") * M_PI / 180.0
                                              : getParam<Real>("value"))
{
}

Real TensorMechanicsHardeningConstant::value(Real /*intnl*/) const { return _val; }

Real TensorMechanicsHardeningConstant::derivative(Real /*intnl*/) const { return 0.0; }

std::string
TensorMechanicsHardeningConstant::modelName() const
{
  return "Constant";
}
