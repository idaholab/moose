#include "ConstantMaterial.h"

template<>
InputParameters validParams<ConstantMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("value", 0., "Constant value being assigned into the property");
  params.addRequiredParam<std::string>("property_name", "The property name to declare");
  return params;
}

ConstantMaterial::ConstantMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _value(getParam<Real>("value")),
    _property(declareProperty<Real>(getParam<std::string>("property_name")))
{
}

ConstantMaterial::~ConstantMaterial()
{
}

void
ConstantMaterial::computeQpProperties()
{
  _property[_qp] = _value;
}
