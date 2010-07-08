#include "Diff2Material.h"

template<>
InputParameters validParams<Diff2Material>()
{
  InputParameters params = validParams<Material>();
  params.set<Real>("diff") = 1.0;
  return params;
}

Diff2Material::Diff2Material(std::string name, MooseSystem & moose_system, InputParameters parameters)
  : Material(name, moose_system, parameters),
    _diff(parameters.get<Real>("diff")),
    _diffusivity(declareProperty<Real>("diff2"))
{
}

void
Diff2Material::computeProperties()
{
  for (unsigned int qp = 0; qp < _n_qpoints; qp++)
    _diffusivity[qp] = _diff;
}
