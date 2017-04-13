/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticitySlipResistanceGSS.h"

template <>
InputParameters
validParams<CrystalPlasticitySlipResistanceGSS>()
{
  InputParameters params = validParams<CrystalPlasticitySlipResistance>();
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
