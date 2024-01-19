#include "ConcreteProperties.h"

registerMooseObject("AppNameApp", ConcreteProperties);

InputParameters
ConcreteProperties::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("");
  return params;
}

ConcreteProperties::ConcreteProperties(const InputParameters & parameters)
  : Material(parameters), _thermal_conductivity(declareMaterialProperty<Real>(...))
{
}

void
ConcreteProperties::computeQpProperties()
{
  _thermal_conductivity[_qp] = 2.25;
}
