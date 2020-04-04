//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPEqvPlasticStrainRate.h"

registerMooseObject("TensorMechanicsApp", HEVPEqvPlasticStrainRate);

InputParameters
HEVPEqvPlasticStrainRate::validParams()
{
  InputParameters params = HEVPInternalVarRateUOBase::validParams();
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
