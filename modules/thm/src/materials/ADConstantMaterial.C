#include "ADConstantMaterial.h"

registerMooseObject("THMApp", ADConstantMaterial);

InputParameters
ADConstantMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("value", 0., "Constant value being assigned into the property");
  params.addRequiredParam<std::string>("property_name", "The property name to declare");
  return params;
}

ADConstantMaterial::ADConstantMaterial(const InputParameters & parameters)
  : Material(parameters),
    _value(getParam<Real>("value")),
    _property_name(getParam<std::string>("property_name")),
    _property(declareADProperty<Real>(_property_name))
{
}

void
ADConstantMaterial::computeQpProperties()
{
  _property[_qp] = _value;
}
