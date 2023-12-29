//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticitySlipResistanceGSS.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticitySlipResistanceGSS);

InputParameters
CrystalPlasticitySlipResistanceGSS::validParams()
{
  InputParameters params = CrystalPlasticitySlipResistance::validParams();
  params.addParam<std::string>("uo_state_var_name",
                               "Name of state variable property: Same as "
                               "state variable user object specified in input "
                               "file.");
  params.addClassDescription("Phenomenological constitutive models' slip resistance base class.  "
                             "Override the virtual functions in your class");
  return params;
}

CrystalPlasticitySlipResistanceGSS::CrystalPlasticitySlipResistanceGSS(
    const InputParameters & parameters)
  : CrystalPlasticitySlipResistance(parameters),
    _mat_prop_state_var(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_state_var_name")))
{
}

bool
CrystalPlasticitySlipResistanceGSS::calcSlipResistance(unsigned int qp,
                                                       std::vector<Real> & val) const
{
  for (unsigned int i = 0; i < _variable_size; ++i)
    val[i] = _mat_prop_state_var[qp][i];

  return true;
}
