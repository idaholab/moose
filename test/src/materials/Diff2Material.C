#include "Diff2Material.h"

template<>
InputParameters validParams<Diff2Material>()
{
  InputParameters params = validParams<Material>();
  params.set<Real>("diff") = 1.0;
  return params;
}

Diff2Material::Diff2Material(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _diff(getParam<Real>("diff")),
    _diffusivity(declareProperty<Real>("diff2")),
    _vector_property(declareProperty<std::vector<Real> >("vector_property"))
{
}

void
Diff2Material::computeQpProperties()
{
  _diffusivity[_qp] = _diff;
  // resize the vector in quadrature point to some random size (10 for instance)
  _vector_property[_qp].resize(10);
}
