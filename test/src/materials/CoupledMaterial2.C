#include "CoupledMaterial2.h"

template<>
InputParameters validParams<CoupledMaterial2>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("mat_prop", "Name of the property this material defines");
  params.addRequiredParam<std::string>("coupled_mat_prop1", "Name of the first property to couple into this material");
  params.addRequiredParam<std::string>("coupled_mat_prop2", "Name of the second property to couple into this material");
  return params;
}


CoupledMaterial2::CoupledMaterial2(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _mat_prop_name(getParam<std::string>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop_name(getParam<std::string>("coupled_mat_prop1")),
    _coupled_mat_prop(getMaterialProperty<Real>(_coupled_mat_prop_name)),
    _coupled_mat_prop_name2(getParam<std::string>("coupled_mat_prop2")),
    _coupled_mat_prop2(getMaterialProperty<Real>(_coupled_mat_prop_name2))
{
}

void
CoupledMaterial2::computeProperties()
{
  for (_qp = 0; _qp < _q_point.size(); ++_qp)
  {
    _mat_prop[_qp] = 4.0/_coupled_mat_prop[_qp]/_coupled_mat_prop2[_qp];       // This will produce a NaN if evaluated out of order
  }
}
