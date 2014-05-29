#include "SumMaterial.h"

template<>
InputParameters validParams<SumMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("sum_prop_name", "The name of the property that holds the summation");
  params.addRequiredParam<std::string>("mp1", "The name of the property that holds the first value");
  params.addRequiredParam<std::string>("mp2", "The name of the property that holds the second value");

  params.addRequiredParam<Real>("val1", "The value of the first property");
  params.addRequiredParam<Real>("val2", "The value of the second property");

  return params;
}

SumMaterial::SumMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _sum(declareProperty<Real>(getParam<std::string>("sum_prop_name"))),
    _mp1(getMaterialProperty<Real>(getParam<std::string>("mp1"))),
    _mp2(getMaterialProperty<Real>(getParam<std::string>("mp2"))),
    _val_mp1(getParam<Real>("val1")),
    _val_mp2(getParam<Real>("val2"))
{
}

SumMaterial::~SumMaterial()
{
}

void
SumMaterial::computeQpProperties()
{
  if (_mp1[_qp] != _val_mp1)
    mooseError("failure");
  if (_mp2[_qp] != _val_mp2)
    mooseError("failure");
  _sum[_qp] = _mp1[_qp] + _mp2[_qp];
}
