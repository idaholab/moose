#include "MTMaterial.h"

template<>
InputParameters validParams<MTMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}

MTMaterial::MTMaterial(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  : Material(name, moose_system, parameters),
    _mat_prop(declareProperty<Real>("matp"))
{
}

void
MTMaterial::computeProperties()
{
  for (unsigned int qp = 0; qp < _n_qpoints; qp++)
    _mat_prop[qp] = _q_point[qp](0) + 1;              // x + 1
}
