#include "VarCouplingMaterialEigen.h"

template<>
InputParameters validParams<VarCouplingMaterialEigen>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addRequiredParam<std::string>("material_prop_name", "Property name");
  return params;
}

VarCouplingMaterialEigen::VarCouplingMaterialEigen(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _var(coupledValue("var")),
    _var_old(coupledValueOld("var")),
    _propname(getParam<std::string>("material_prop_name")),
    _mat(declareProperty<Real>(_propname)),
    _mat_old(declareProperty<Real>(_propname+"_old"))
{
}

void
VarCouplingMaterialEigen::computeQpProperties()
{
  _mat[_qp] = _var[_qp];
  _mat_old[_qp] = _var_old[_qp];
}
