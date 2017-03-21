/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPEqvPlasticStrainRate.h"

template <>
InputParameters
validParams<HEVPEqvPlasticStrainRate>()
{
  InputParameters params = validParams<HEVPInternalVarRateUOBase>();
  params.addParam<Real>("h_scaling", 1.0, "Scaling parameter");
  params.addClassDescription("User Object computing equivalent plastic strain rate");
  return params;
}

HEVPEqvPlasticStrainRate::HEVPEqvPlasticStrainRate(const InputParameters & parameters)
  : HEVPInternalVarRateUOBase(parameters), _h(getParam<Real>("h_scaling"))
{
}

bool
HEVPEqvPlasticStrainRate::computeValue(unsigned int qp, Real & val) const
{
  val = _h * _flow_rate[qp];
  return true;
}

bool
HEVPEqvPlasticStrainRate::computeDerivative(unsigned int /*qp*/,
                                            const std::string & coupled_var_name,
                                            Real & val) const
{
  val = 0;

  if (_flow_rate_prop_name == coupled_var_name)
    val = _h;

  return true;
}
