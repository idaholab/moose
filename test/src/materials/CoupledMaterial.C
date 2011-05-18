#include "CoupledMaterial.h"

template<>
InputParameters validParams<CoupledMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("mat_prop", "Name of the property to couple into this material");
  return params;
}


CoupledMaterial::CoupledMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _my_prop(declareProperty<Real>("some_prop")),
    _mat_prop_name(getParam<std::string>("mat_prop")),
    _coupled_mat_prop(getMaterialProperty<Real>(_mat_prop_name))
{
}

void
CoupledMaterial::computeProperties()
{
  for (int qp = 0; qp < _q_point.size(); ++qp)
  {
    _my_prop[_qp] = _coupled_mat_prop[_qp] + 1.0;
  }
}
