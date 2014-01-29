#include "CoupledMaterial.h"

template<>
InputParameters validParams<CoupledMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("mat_prop", "Name of the property this material defines");
  params.addRequiredParam<std::string>("coupled_mat_prop", "Name of the property to couple into this material");
  params.addParam<bool>("use_old_prop", false, "Boolean indicating whether to use the old coupled property instead of the current property");
  return params;
}


CoupledMaterial::CoupledMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _mat_prop_name(getParam<std::string>("mat_prop")),
    _mat_prop(declareProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop_name(getParam<std::string>("coupled_mat_prop")),
    _coupled_mat_prop(getParam<bool>("use_old_prop") ? getMaterialPropertyOld<Real>(_coupled_mat_prop_name) : getMaterialProperty<Real>(_coupled_mat_prop_name))
{
}

void
CoupledMaterial::computeProperties()
{
  for (_qp = 0; _qp < _q_point.size(); ++_qp)
  {
    _mat_prop[_qp] = 4.0/_coupled_mat_prop[_qp];       // This will produce a NaN if evaluated out of order
  }
}
