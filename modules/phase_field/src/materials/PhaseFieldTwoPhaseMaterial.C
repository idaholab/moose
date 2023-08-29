/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseFieldTwoPhaseMaterial.h"

registerADMooseObject("PhaseFieldApp", PhaseFieldTwoPhaseMaterial);

InputParameters
PhaseFieldTwoPhaseMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addRequiredCoupledVar("pf", "Phase field variable");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name of material property.");
  params.addRequiredParam<Real>("prop_value_1", "value of phase 1.");
  params.addRequiredParam<Real>("prop_value_2", "value of phase 2.");
  return params;
}

PhaseFieldTwoPhaseMaterial::PhaseFieldTwoPhaseMaterial(
    const InputParameters & parameters)
  : ADMaterial(parameters),
    _pf(adCoupledValue("pf")),
    _prop_value_1(getParam<Real>("prop_value_1")),
    _prop_value_2(getParam<Real>("prop_value_2")),
    _prop(declareADProperty<Real>(getParam<MaterialPropertyName>("prop_name")))
{
}


void
PhaseFieldTwoPhaseMaterial::computeQpProperties()
{
  _prop[_qp] = 0.5*(1-_pf[_qp])*_prop_value_1 + 0.5*(1+_pf[_qp])*_prop_value_2;
}
