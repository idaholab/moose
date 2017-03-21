/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "SumMaterial.h"

template <>
InputParameters
validParams<SumMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>(
      "sum_prop_name", "The name of the property that holds the summation");
  params.addRequiredParam<MaterialPropertyName>(
      "mp1", "The name of the property that holds the first value");
  params.addRequiredParam<MaterialPropertyName>(
      "mp2", "The name of the property that holds the second value");

  params.addRequiredParam<Real>("val1", "The value of the first property");
  params.addRequiredParam<Real>("val2", "The value of the second property");

  return params;
}

SumMaterial::SumMaterial(const InputParameters & parameters)
  : Material(parameters),
    _sum(declareProperty<Real>(getParam<MaterialPropertyName>("sum_prop_name"))),
    _mp1(getMaterialProperty<Real>("mp1")),
    _mp2(getMaterialProperty<Real>("mp2")),
    _val_mp1(getParam<Real>("val1")),
    _val_mp2(getParam<Real>("val2"))
{
}

SumMaterial::~SumMaterial() {}

void
SumMaterial::computeQpProperties()
{
  if (_mp1[_qp] != _val_mp1)
    mooseError("failure");
  if (_mp2[_qp] != _val_mp2)
    mooseError("failure");
  _sum[_qp] = _mp1[_qp] + _mp2[_qp];
}
