//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPEqvPlasticStrain.h"

registerMooseObject("TensorMechanicsApp", HEVPEqvPlasticStrain);

InputParameters
HEVPEqvPlasticStrain::validParams()
{
  InputParameters params = HEVPInternalVarUOBase::validParams();
  params.addClassDescription("User Object to integrate equivalent plastic strain");
  return params;
}

HEVPEqvPlasticStrain::HEVPEqvPlasticStrain(const InputParameters & parameters)
  : HEVPInternalVarUOBase(parameters)
{
}

bool
HEVPEqvPlasticStrain::computeValue(unsigned int qp, Real dt, Real & val) const
{
  val = _this_old[qp] + _intvar_rate[qp] * dt;
  return true;
}

bool
HEVPEqvPlasticStrain::computeDerivative(unsigned int /*qp*/,
                                        Real dt,
                                        const std::string & coupled_var_name,
                                        Real & val) const
{
  val = 0;

  if (_intvar_rate_prop_name == coupled_var_name)
    val = dt;

  return true;
}
