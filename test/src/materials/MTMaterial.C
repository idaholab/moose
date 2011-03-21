#include "MTMaterial.h"

template<>
InputParameters validParams<MTMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("value", 1.0, "lift");
  return params;
}

MTMaterial::MTMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _mat_prop(declareProperty<Real>("matp")),
    _value(getParam<Real>("value"))
{
}

void
MTMaterial::computeQpProperties()
{
  _mat_prop[_qp] = _q_point[_qp](0) + _value;              // x + value
}
