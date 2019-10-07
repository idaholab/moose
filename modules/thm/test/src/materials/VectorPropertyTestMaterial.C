#include "VectorPropertyTestMaterial.h"

registerMooseObject("THMTestApp", VectorPropertyTestMaterial);

template <>
InputParameters
validParams<VectorPropertyTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Test material with vector properties");
  return params;
}

VectorPropertyTestMaterial::VectorPropertyTestMaterial(const InputParameters & parameters)
  : Material(parameters), _vec(declareProperty<std::vector<Real>>("test_property"))
{
}

void
VectorPropertyTestMaterial::computeQpProperties()
{
  _vec[_qp].resize(5);
  for (std::size_t i = 0; i < _vec[_qp].size(); i++)
    _vec[_qp][i] = i * 2.;
}
